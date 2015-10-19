#include <stdint.h>
#include <string.h>
#include <io.h>
#include <signal.h>
#include "uif.h"
#include "uif_macros.h"

void panic(void) __attribute__ ((noreturn));

/* ------------------------------------------------------------ */

void msleep(int n)
{
  /* 
   *  It takes 6 clocks for one iteration of inner loop.
   *  N loop takes 6*N clocks = 6*N*1/MCLK seconds.
   *  1ms = 1/1000 s = 6/MCLK * N, N=MCLK/6000 ~ 1333
   */
  volatile int i;
  while (n-- > 0)
    for (i=MCLK/6000; i > 0; i--) ;
}

void usleep(int n)
{
  while (n--) { nop(); nop(); }
}

void uif_reset_tusb3410()
{
  uif_assert_RST_3410();
  msleep(10);
  uif_negate_RST_3410();
  msleep(500);
}

/* ------------------------------------------------------------ */

static void setup_system_clock()
{
  /*
   *   MCLK   = XT2 = 12MHz -- for CPU
   *   SMCLK  = XT2 = 12MHz -- for I2C, USART
   */
  uif_assert_RST_3410();
  {
    BCSCTL1 = 0;
    do {
      volatile int    i;

      IFG1 &= ~OFIFG;
      for (i=0xff; i > 0; i--) ;

    } while ((IFG1&OFIFG) != 0);

    /* MCLK=XT2, SMCLK=XT2, MCLK:SMCLK=1:1 */
    BCSCTL2 = (SELM_XT2CLK|DIVM_DIV1|SELS|DIVS_DIV1);
  }
  uif_negate_RST_3410();
  msleep(700);   /* Wait for TUSB3410 to boot */
}

static void setup_io_ports()
{
  P1OUT = 0;  P2OUT = 0;  P3OUT = 0;
  P4OUT = 0;  P5OUT = 0;  P6OUT = 0;

  /* P1.3  URTS I
     P1.4  UDTR I
     P1.5  UDSR O
     P1.6  UCTS O 
  */
  P1OUT = (BIT6|BIT5); 
  P1DIR = (BIT6|BIT5);
  P1SEL = 0;

  /* Port 2: unused */
  P2OUT = P2DIR = P2SEL = 0;

  /* Port 3: USART0 to TUSB3410 
    P3.0:     N.C.
    P3.1:     SDA   (EEPROM, Pulled up by 1.5K)
    P3.2:     N.C.
    P3.3:  O  SCL   (EEPROM, Pulled up by 1.5K)
    P3.4:  O  UTXD  (To TUSB3410)
    P3.5:  I  URXD  (From TUSB3410)
    P3.6:  O  BTXD  (Target bord, not used for now)
    P3.7:  I  BRXDI (Target bord, not used for now)
  */
  /* BTXD(BIT6) is used as RESET_N for cc8051 */
  P3SEL = (BIT5|BIT4|BIT3|BIT1);
  P3DIR = (BIT6|BIT4|BIT3|BIT1);

  /* P4.6: RST3410  -- see reset_tusb3410()
   */
  P4OUT= P4SEL = P4DIR = 0;
  
  /* P5.1 O TDI (SBWTDIO)
     P5.2 I TDO (SBWTDIO)
     P5.3 O TCK (SBWTCK) 
     P5.5 O SMCLK (to TUSB3410)
  */
  P5SEL = BIT5;
  P5DIR = (BIT3|BIT1);

  /* Port 6: unused */
  P6SEL = 0;
  P6DIR = 0;
}

#define MSGBUFSIZE 32
static char   msgbuf[MSGBUFSIZE];
static int    mp;

void init_msgbuf()
{
  memset(msgbuf, 0, MSGBUFSIZE);
  mp = 0;
}

void led_message(char *s)
{
  int   len = strlen(s);
  if (mp + len + 1 > MSGBUFSIZE) panic();
  strcpy(&msgbuf[mp], s);
  mp += len + 1;
}

void panic()
{
  while (1) ;
}

/* ---------------------------------------------------- */

void uif_attach_jtag()
{
  /* 4 Wire JTAG is not supported on this machine */
} 

void uif_detach_jtag()
{
  /* 4 Wire JTAG is not supported on this machine */
}

void uif_attach_sbw()
{
  /* JTAG - Spy-Bi-Wire entry sequence (8.3.1 of slau265e) */

  set_TDI();
  set_TCK();
  msleep(10);
  clr_TCK();
  set_TCK();

  /*
  clr_TCK();
  set_TCK();
  */
}

void uif_detach_sbw()
{
  /* nothing to do */
}

 /* RESET_N for cc8051 */
#define set_RESET_N()      (P3OUT|=BIT6)   
#define clr_RESET_N()      (P3OUT&=~BIT6)

void uif_attach_cc8051()
{
  clr_TCK();
  set_RESET_N();
  clr_RESET_N();
  msleep(1);
  set_TCK();    /* first rising edge */
  clr_TCK();
  set_TCK();    /* second rising edge */
  clr_TCK();
  nop();
  set_RESET_N();
}

int uif_set_vcct(int mV)
{
  return 3600;
}

void uif_reset_target()
{
  /* uif_attach_sbw() will take care 
     of this function */
}

void uif_assert_RST_3410()
{
#ifdef EZ430U1P1
  /* eZ430-F2013 (SLAU176B) */
  P3OUT &= ~BIT6;
  P3DIR |= BIT6;
#else
  /* Chronos, RF2500 (SLAU292) */
  P4OUT &= ~BIT6;
  P4DIR |= BIT6;
#endif
}

void uif_negate_RST_3410()
{
#ifdef EZ430U1P1
  /* eZ430-F2013   (SLAU176B)  */
  P3OUT |= BIT6;
  P3DIR &= ~BIT6;
#else
  /* Chronos, RF2500 (SLAU292) */
  P4OUT |= BIT6;
  P4DIR &= ~BIT6;
#endif
}

/* ------------------------------------------------------------------------- */

#define EEPROM_ADDRESS           0x50       // Address of EEPROM
#define OWN_ADDRESS              0x40       // I2C own address

void uif_usart_I2C()
{
  dint();
  U0CTL = SWRST;                            // USART logic in reset state
  U0CTL &= ~I2CEN;                          // Clear I2CEN bit
  {
    U0CTL   = (I2C|SYNC|MST);               // I2C master mode, 7-bit addr
    I2CTCTL = (I2CTRX|I2CSSEL_SMCLK);       // Byte mode, repeat mode, SMCLK
    I2CSA   = EEPROM_ADDRESS;               // I2C slave address
    I2COA   = OWN_ADDRESS;                  // Own address
    I2CPSC  = 3;                            // I2C prd = 4 * clock prd
    I2CSCLH = 8;                            // SCL high prd = 10 * I2C prd
    I2CSCLL = 8;                            // SCL low prd = 10 * I2C prd
  }
  U0CTL |= I2CEN;                           // Enable I2C module operation
  eint();
}

void uif_usart_com()
{
#define BAUDMOD      0x00  /* Should be zero, since we are on the same clock */
  dint();
  U0CTL  = 0;
  U0CTL  = SWRST;          /* Softwawre reset */ 
  {
    ME1   |= (UTXE0|URXE0);
    U0CTL |= CHAR;
    U0TCTL = SSEL_SMCLK;
    U0BR0  =  MCLK/TUSB3410_BAUDRATE;
    U0BR1  = (MCLK/TUSB3410_BAUDRATE)>>8;
    U0MCTL = BAUDMOD;
  }
  U0CTL &= ~SWRST;                      
  IE1   = URXIE0;
  eint();
}

void init_uif()
{
  init_msgbuf();
  setup_io_ports();
  setup_system_clock();
  uif_usart_com();
}

/* EOF */
