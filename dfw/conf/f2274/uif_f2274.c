#include <stdint.h>
#include <string.h>
#include <io.h>
#include <signal.h>
#include "uif.h"
#include "uif_macros.h"
#include "misc.h"

/* ------------------------------------------------------------ */

static void calibrate_dco()
{
  /*
      not implemented yet.

      1) Setup CC2500's GDO0 to output CLK.
      2) Setup F2274's timer to use ACLK(XIN).
      3) Syncronize DCO and TIMER.

      or use 13MHz or 6.5MHz from CC2500 as MCLK.

      panic for now.
  */
  power_LED_on();
  mode_LED_off();
  while (1) {
    usleep(30000);
    power_LED_flip();
    mode_LED_flip();
  }
}

static void setup_system_clock()
{
#if MCLK != 12000000
#error "MCLK has to be set 12000000"
#endif
  /*
   *    MCLK  = XT2 = 12MHz -- for CPU
   *    SMCLK = XT2 = 12MHz -- for I2C, USART
   */
  if (CALDCO_12MHZ != 0xff && CALBC1_12MHZ != 0xff) {
    DCOCTL  = CALDCO_12MHZ;
    BCSCTL1 = CALBC1_12MHZ;
  } else {
    calibrate_dco();
  }
  /* MCLK=DCO, SMCLK=DCO, MCLK:SMCLK=1:1, Internal resistor */
  BCSCTL2 = 0;
  msleep(700);
}

/*
 *  RF2500T:
 *
 *                TP HEADER
 *      GND    --- P1   P2 ---   VCC_EXT
 *      P2.0   --- P3   P4 ---   P2.1
 *      P2.2   --- P5   P6 ---   P2.3
 *      P2.4   --- P7   P8 ---   P4.3
 *      P4.4   --- P9  P10 ---   P4.5
 *      P4.6   --- P11 P12 ---   GND
 *      GDO0   --- P13 P14 ---   GDO2
 *      P3.2   --- P15 P16 ---   P3.3
 *      P3.0   --- P17 P18 ---   P3.1
 *
 *  NOTE:
 *     1)  PORT3 is used for CC2500
 *     2)  It might be covenient to leave PORT2.0/PORT2.1 
 *         (MCLK/ACLK) open, so that F2274 can provide clock 
 *         for the other party.
 *
 *                 4Wire mode   Spi-Bi-Wire mode    CC8051
 *     PORT2.2  I      TDO       ---+--- SBWTDIO    Debug Data
 *     PORT2.4  O      TDI       ---J
 *     PORT4.4  O      TMS  
 *     PORT4.6  O      TCK       ------- SBWTCK     Debug TCLK
 *     PORT4.3  O     TGTRST                        RESET_N
 *     PORT4.5  O     TEST   
 *
 *      We need a jumper when we do SBW
 */
#define set_TGTRST()       (P4OUT|=BIT3)
#define clr_TGTRST()       (P4OUT&=~BIT3)
#define set_TEST()         (P4OUT|=BIT5)
#define clr_TEST()         (P4OUT&=~BIT5)
#define hiz_TEST()         (P4DIR&=~BIT5)

static void setup_io_ports()
{
  /* LED */
  P1OUT &= ~(BIT1|BIT0);
  P1DIR |=  (BIT1|BIT0);

  /* JTAG/SBW */
  P2OUT  =   0; /* (BIT4); */
  P2DIR |= (BIT4);
  P4OUT  =  0; /* (BIT6|BIT5|BIT4|BIT3); */
  P4DIR |= (BIT6|BIT5|BIT4|BIT3);

  /* UCI */
  P3SEL |= (BIT5|BIT4);
}

/* ---------------------------------------------------- */

void uif_usart_com()
{
  /*
   *   XXX: How fast can we go with DCO 12MHz???
   *   12MHz SMCLK. 
   *
   *   115200Baud : UCBRx= 6, UCBRSx=0, UCBRFx=8, UCOS16=1.
   *     9600Baud : UCBRx=78, UCBRSx=0, UCBRFx=2, UCOS16=1.
   *
   *   c.f.  Table 15-5 of SLAU144E.
   *
   *   From F2274 datasheet.
   *    max f_UCSI   = f_SYSTEM
   *    max f_BITCLK = 1MHz
   */
  dint();
  UCA0CTL0  =  0; /* 8bit, non-parity, LSB first, 1 stop bit, UART, Async */
  UCA0CTL1  =  UCSSEL_SMCLK; /* 12MHz DCO */
  UCA0BR0   =  78;
  UCA0BR1   =  0;
  UCA0MCTL  =  (UCBRF_2|UCBRS_0|UCOS16);                       
  UCA0CTL1 &= ~UCSWRST;
  IE2      |=  UCA0RXIE;                          
  eint();
}

void uif_attach_jtag()
{
  //set_SELT();
  //set_TDION();        
  //clr_ENI2O();
  //msleep(10);

  /* JTAG entry sequence (8.3.1 of slau265e) */

  set_TGTRST();
  clr_TEST();
  usleep(800);           /* delay min 800us - clr SBW controller */

  set_TEST();
  usleep(50); 
  clr_TGTRST();
  usleep(100);
  clr_TEST();
  usleep(5);
  set_TEST();
  usleep(5);
  set_TGTRST();
  msleep(10);
} 

void uif_detach_jtag()
{
  //clr_SELT();
  //clr_TDION();        
}

void uif_attach_sbw()
{
  hiz_TEST();

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
#define set_RESET_N()      set_TGTRST()
#define clr_RESET_N()      clr_TGTRST()

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

/* ------------------------------------------------------------ */

void msleep(int n)
{
  /* 
   *   It takes 6 clocks for one iteration of inner loop.
   *   N loop takes 6*N clocks = 6*N*1/MCLK seconds.
   *   1ms = 1/1000 s = 6/MCLK * N, N=MCLK/6000 ~ 1333
   */
  volatile int   i;

  while (n-- > 0)
    for (i=MCLK/6000; i > 0; i--) ;
}

void usleep(int n)
{
  while (n--) { nop(); nop(); }
}

/* ---------------------------------------------------- */

#define SPEED    8              /* smaller faster */

void led_message(char *s)
{
  int      c;
  dint();
  power_LED_off();
  msleep(70*SPEED);
  while ((c=*s++) != '\0') {
    switch (c) {
    case '.':
      power_LED_on();
      msleep(10*SPEED);
      power_LED_off();
      msleep(10*SPEED);
      break;
    case '-':
    case '_':
      power_LED_on();
      msleep(25*SPEED);
      power_LED_off();
      msleep(10*SPEED);
      break;
    case ' ':
      msleep(30*SPEED);
      break;
    default: ;
    }
  }
  msleep(70*SPEED);
  power_LED_on();
  msleep(140*SPEED);
  eint();
}

void panic()
{
  while (1) led_message("... --- ...");
}

/* ------------------------------------------------------------------------- */

void init_uif()
{
  setup_io_ports();
  setup_system_clock();
  uif_usart_com();
}

/* EOF */
