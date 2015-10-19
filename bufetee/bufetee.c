/* -*- coding: utf-8 -*-
 * 
 *  bufetee.c :  Backup FET' eeprom.
 *
 *  References:
 *   http://en.wikipedia.org/wiki/I²C
 *   Microchip 24AA128/24LC128/24FC128 data sheet.
 */
#include <io.h>
#include <stdint.h>

#define ram __attribute__((section(".data")))

asm(".data                \n"
"start:                   \n"
"    mov #__stack, r1     \n"
"    br  #bufetee         \n"
"restore:                 \n"
"    mov #__stack, r1     \n"
"    br  #refetee         \n"
"ee_addr: .short 0        \n"
"ee_buf:  .short 0        \n"
"ee_len:  .short 0        \n"
"ee_err:  .short 0        \n"
"err:     .short 0        \n"
);

extern uint16_t  ee_addr;
extern uint8_t  *ee_buf;
extern uint16_t  ee_len;
extern uint16_t  ee_err;
extern int       err;

/*
 *  bit banging I2C master 
 *
 *   Derived from sample code shown in
 *   http://en.wikipedia.org/wiki/I²C
 */

/*   PORT configuration 
 *
 *   I2C bus is driven by open drain (collector) output.
 *   So in most cases, pull-up resister of 1K~50K Ohm is 
 *   requied for each SCL/SDA signals. 
 *
 *   Wii's nuchuk has 1.8K Ohm pull-up resister built in, 
 *   i.e., about 2mA driving current required and this
 *   is within MSP430's limit so we can connect MSP430
 *   and nunchuk directly.
 */
#define I2COUT      P3OUT
#define I2CIN       P3IN
#define I2CDIR      P3DIR
#define I2CSEL      P3SEL

#define SDABIT      BIT1
#define SCLBIT      BIT3

/*   DELAY_COUNT determins SCL pulse width by busy wait loop. 
 *   Good amount depends on CPU's clock and the device to 
 *   communicate with.
 *   For nunchuk, 10 turned out good for 6MHz CPU operation, 
 *   with VDD=2.9V.  5 is OK when VDD is more than 3.3V.
 *   For EEPROM on eZ430 with DCO clock 2 looks OK, but not
 *   not tested well.
 */
#define DELAY_COUNT 2

ram void i2c_init()
{
  err     = 0;
  ee_err  = 0;
  
  I2COUT  &= ~(SDABIT|SCLBIT);
  I2CDIR  &= ~(SDABIT|SCLBIT);
  I2CSEL  &= ~(SDABIT|SCLBIT);
}

ram static inline void set_SDA()   { I2CDIR &=  ~SDABIT; }
ram static inline void clr_SDA()   { I2CDIR |=   SDABIT; }
ram static inline int  tst_SDA()   { return (I2CIN&SDABIT) ? 1 : 0;}

ram static inline void set_SCL()   { I2CDIR &=  ~SCLBIT; }
ram static inline void clr_SCL()   { I2CDIR |=   SCLBIT; }
ram static inline int  tst_SCL()   { return (I2CIN&SCLBIT) ? 1 : 0;}

ram static inline void i2c_delay()
{ 
  volatile uint16_t n = DELAY_COUNT;
  while (n-- > 0) ;
}

ram static inline unsigned read_bit()
{
  unsigned bit;

  set_SDA();
  i2c_delay();
  set_SCL();
  while (!tst_SCL()) ;
  bit = tst_SDA();
  i2c_delay();
  clr_SCL();
  i2c_delay();
  return bit;
}

ram static inline void write_bit(unsigned bit)
{
  if (bit) set_SDA(); else clr_SDA();
  i2c_delay();
  set_SCL();
  while (!tst_SCL()) ;
  i2c_delay();
  clr_SCL();
  i2c_delay();
}

ram int  i2c_errno()     {return err;}
ram void i2c_clr_errno() {err=0;}

ram void i2c_start()
{
  set_SDA();
  set_SCL();
  if (!tst_SDA()) {
    err = 1;
    return;
  }
  i2c_delay();
  clr_SDA();
  i2c_delay();
  clr_SCL();
  i2c_delay();
  err = 0;
}

ram void i2c_stop()
{
  clr_SDA();
  i2c_delay();
  set_SCL();
  while (!tst_SCL()) ;

  set_SDA();
  if (!tst_SDA()) {
    err = 2;
  }
  i2c_delay();
}

ram unsigned i2c_write(uint8_t x)
{
  int        i;
  
  for(i = 0; i < 8; i++) {
    write_bit(x&0x80);
    x  <<= 1;
  }
  return read_bit();
}

ram uint8_t i2c_read(int ack)
{
  int        i;
  uint8_t    x;
  
  x = 0;
  for (i = 0; i < 8; i++) {
    x <<= 1;
    x |= read_bit();
  }
  write_bit(ack);
  return x;
}

ram void i2c_cmd(uint8_t cmd)
{
  while (1) {
    i2c_start();
    if (!i2c_write(cmd))
      break;
    i2c_stop();
    i2c_delay();
  }
}

ram read_ee(uint16_t addr, uint8_t *buf, uint16_t len)
{
  i2c_start();
  i2c_write(0xa0);
  i2c_write(addr>>8);
  i2c_write(addr);
  i2c_start();
  i2c_write(0xa1);
  while (len-- > 1)
    *buf++ = i2c_read(0);
  *buf++ = i2c_read(1);
  i2c_stop();
}

/*
 *    addr has to point EEPROM's physical page boundary. (64 bytes).
 *    len must be 64 unless it is the final page.
 *    c.f. Sec. 6.3 of 24LC128 data sheet.
 */
ram write_ee(uint16_t addr, uint8_t *buf, uint16_t len)
{
  do {
    /*
     *   Acknowledge polling. c.f.,
     *   Fig.7-1 of 24LC128 data sheet.
     */
    i2c_start();
  } while (i2c_write(0xa0));
  i2c_write(addr>>8);
  i2c_write(addr);
  while (len-- > 0)
    i2c_write(*buf++);
  i2c_stop();
}

ram bufetee()
{
  WDTCTL = (WDTPW|WDTHOLD);

  i2c_init();
  read_ee(ee_addr, ee_buf, ee_len);

  while (1) {
    LPM0;
    nop();
  }
}

ram refetee()
{
  WDTCTL = (WDTPW|WDTHOLD);

  i2c_init();
  write_ee(ee_addr, ee_buf, ee_len);

  while (1) {
    LPM0;
    nop();
  }
}

/* EOF */
