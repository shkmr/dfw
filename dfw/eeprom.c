/* -*- coding: utf-8 -*- 
 */
#include <io.h>
#include <stdint.h>
#include <string.h>
#include "uif.h"
#include "misc.h"

static const uint8_t FET430UIF_EEPROM[] = {
#ifdef TUSB3410_USB_HEADER_ONLY
#include "../tusb3410_descriptor_header.inc"
#else
#include "../tusb3410_autoexec_firmware.inc"
#endif
};

void i2c_init(void);

void eeprom_read(  uint16_t addr,       uint8_t *buf, uint16_t len);
void eeprom_write( uint16_t addr, const uint8_t *buf, uint16_t len);
int  eeprom_verify(uint16_t addr, const uint8_t *buf, uint16_t len);

void eeprom_firmware_check()
{
  i2c_init();
  if (eeprom_verify(0, FET430UIF_EEPROM, sizeof(FET430UIF_EEPROM))) {
    led_message(". .  -. ---"); 
    uif_assert_RST_3410();
    eeprom_write(0, FET430UIF_EEPROM, sizeof(FET430UIF_EEPROM));
    led_message("."); 
    uif_reset_tusb3410();
  }
}

/*
 *  bit banging I2C master 
 *
 *   Derived from sample code shown in
 *   http://en.wikipedia.org/wiki/I²C
 *
 *  References:
 *   http://en.wikipedia.org/wiki/I²C
 *   Microchip 24AA128/24LC128/24FC128 data sheet.
 */

static int err;

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
#define DELAY_COUNT 20

void i2c_init()
{
  err     = 0;
  
  I2COUT  &= ~(SDABIT|SCLBIT);
  I2CDIR  &= ~(SDABIT|SCLBIT);
  I2CSEL  &= ~(SDABIT|SCLBIT);
}

static inline void set_SDA()   { I2CDIR &=  ~SDABIT; }
static inline void clr_SDA()   { I2CDIR |=   SDABIT; }
static inline int  tst_SDA()   { return (I2CIN&SDABIT) ? 1 : 0;}

static inline void set_SCL()   { I2CDIR &=  ~SCLBIT; }
static inline void clr_SCL()   { I2CDIR |=   SCLBIT; }
static inline int  tst_SCL()   { return (I2CIN&SCLBIT) ? 1 : 0;}

static inline void i2c_delay()
{ 
  volatile uint16_t n = DELAY_COUNT;
  while (n-- > 0) ;
}

static inline unsigned read_bit()
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

static inline void write_bit(unsigned bit)
{
  if (bit) set_SDA(); else clr_SDA();
  i2c_delay();
  set_SCL();
  while (!tst_SCL()) ;
  i2c_delay();
  clr_SCL();
  i2c_delay();
}

int  i2c_errno()     {return err;}
void i2c_clr_errno() {err=0;}

void i2c_start()
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

void i2c_stop()
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

unsigned i2c_write(uint8_t x)
{
  int        i;
  
  for(i = 0; i < 8; i++) {
    write_bit(x&0x80);
    x  <<= 1;
  }
  return read_bit();
}

uint8_t i2c_read(int ack)
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

void i2c_cmd(uint8_t cmd)
{
  while (1) {
    i2c_start();
    if (!i2c_write(cmd))
      break;
    i2c_stop();
    i2c_delay();
  }
}

/* ------------------------------------------------------------------------- */

void read_ee(uint16_t addr, uint8_t *buf, uint16_t len)
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
void write_ee(uint16_t addr, const uint8_t *buf, uint16_t len)
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

/* ------------------------------------------------------------------------- */

enum MICROCHIP_24LC128 {
  EEPROM_PAGE_SIZE  = 64
};

void eeprom_read(uint16_t addr, uint8_t *buf, uint16_t len)
{
  while (len > 64) {
    read_ee(addr, buf, EEPROM_PAGE_SIZE);
    addr += EEPROM_PAGE_SIZE;
    buf  += EEPROM_PAGE_SIZE;
    len  -= EEPROM_PAGE_SIZE;
  }
  read_ee(addr, buf, len);
}

int eeprom_verify(uint16_t addr, const uint8_t *buf, uint16_t len)
{
  uint8_t   rb[EEPROM_PAGE_SIZE];

  while (len > EEPROM_PAGE_SIZE) {
    read_ee(addr, rb, EEPROM_PAGE_SIZE);
    if (memcmp(rb, buf, EEPROM_PAGE_SIZE))
      return 1;
    addr += EEPROM_PAGE_SIZE;
    buf  += EEPROM_PAGE_SIZE;
    len  -= EEPROM_PAGE_SIZE;
  }
  read_ee(addr, rb, len);
  if (memcmp(rb, buf, len)) 
    return 1;
  return 0;
}

/*
 *    eeprom_write(): addr has to be EEPROM's page boundary
 */
void eeprom_write(uint16_t addr, const uint8_t *buf, uint16_t len)
{
  while (len > 64) {
    write_ee(addr, buf, EEPROM_PAGE_SIZE);
    addr += EEPROM_PAGE_SIZE;
    buf  += EEPROM_PAGE_SIZE;
    len  -= EEPROM_PAGE_SIZE;
  }
  write_ee(addr, buf, len);
}

/* EOF */
