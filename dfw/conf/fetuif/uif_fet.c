/* FET430UIF lowlevel hardware access */

#include <stdint.h>
#include <io.h>
#include <signal.h>
#include "uif.h"
#include "uif_macros.h"

#define set_SELT_()         (P2OUT|=BIT5)
#define clr_SELT_()         (P2OUT&=~BIT5)
#define tst_SELT_()         (P2OUT&BIT5)
#define set_SELT()          clr_SELT_()
#define clr_SELT()          set_SELT_()
#define tst_SELT()          (!tst_SELT_())

#define set_TGTRST()        (P2OUT|=BIT6)
#define clr_TGTRST()        (P2OUT&=~BIT6)
#define tst_TGTRST()        (P2OUT&BIT6)

#define set_TEST_()         (P4OUT|=BIT0)
#define clr_TEST_()         (P4OUT&=~BIT0)
#define tst_TEST_()         (P4OUT&BIT0)
#define set_TEST()          clr_TEST_()
#define clr_TEST()          set_TEST_()
#define tst_TEST()          (!tst_TEST_())

#define set_VCCTON()        (P4OUT|=BIT3)
#define clr_VCCTON()        (P4OUT&=~BIT3)
#define tst_VCCTON()        (P4OUT&BIT3)

#define set_TDION()         (P4OUT|=BIT4)
#define clr_TDION()         (P4OUT&=~BIT4)
#define tst_TDION()         (P4OUT&BIT4)

#define set_VF2TDI()        (P4OUT|=BIT5)
#define clr_VF2TDI()        (P4OUT&=~BIT5)
#define tst_VF2TDI()        (P4OUT&BIT5)

#define set_VF2TEST()       (P4OUT|=BIT5)
#define clr_VF2TEST()       (P4OUT&=~BIT5)
#define tst_VF2TEST()       (P4OUT&BIT6)


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

/* --------------------------------------------------------------- */

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

/* ------------------------------------------------------------ */

static void setup_system_clock()
{
  /* 
     MCLK = XT1 = 8MHz -- for CPU
     ACLK = XT1 = 8MHz -- I2C, USART
  */
  BCSCTL1 |= (XT2OFF|XTS|DIVA_DIV1);
  _BIC_SR(OSCOFF);           /* Turn ON XT1 */
  do {
    volatile int   i;
    IFG1 &= ~OFIFG;
    for (i = 0xff; i>0; i--) ;
  } while ((IFG1&OFIFG) != 0);
  BCSCTL2 |= SELM_LFXTCLK;   /* MCLK=XT1 */
}

/* ------------------------ A D C ----------------------------- */
/*
  Connection               LSB=2.5V/4095

   VCCI --- 30k -+--- ADC0
                 |
		22k              ADC0*LSB = 22/52*VCCI
                 |
                GND              VCCI = ADC0*(2500mV*52)/(4095*22) = 1443uV*ADC0

   VCCT --- 30k -+--- ADC1
     |           |
     |          22k              ADC1*LSB = 22/52*VCCT
    1.5          |
     |           GND 
     |
   VCCR --- 30k -+--- ADC3
                 |
		22k              ADC3*LSB = 22/52*VCCR
                 |            
                GND 
                                 TAGET_CURRENT = (VCCR-VCCT)/1.5Ohm


     VF --- 39k -+--- ADC2
                 |
		22k              ADC2*LSB = 22/61*VF
                 |              
                GND              VF = ADC2*(2500mV*61)/(4096*22) = 1692uV*ADC2


*/
#define ADC_LSB_uV   610
#define VF_LSB_uV   1692
#define VCC_LSB_uV  1443

static void setup_adc()
{
 /* Single Conv, 5MHz ADC clock.
    2.5V internal reference.
    192 clock or 38us sample time. */

  ADC12CTL0 &= ~ENC;
  ADC12CTL0 = (ADC12ON|REFON|REF2_5V|SHT0_DIV192); 
  ADC12CTL1 = SHP;
}

int adc(int n) /* 2.5V full scale 12 bit ADC */
{
  if ((0 <= n) && (n < 16)) {
    ADC12CTL0  &= ~ENC;           
    ADC12MCTL0  = 0x10 + n; /* EOS=0, SREF0=1(VREF&AVSS) */
    ADC12CTL0  |= ENC;           
    ADC12CTL0  |= ADC12SC;       
    while ((ADC12IFG&BIT0) == 0);
    return ADC12MEM0;
  } else
    return 0;
}

/* ----------------------- D A C --------------------------------- */
/*
                    VCCT 
                      |
                     60k
                      |
 (TPS76601D) FB ------+ 
                      |
                      |
        SETVCCT--39k--+--30k-- GND

	FB=1.224V

  (FB - VCCT)/60k + (FB-SETVCCT)/39k + FB/30k = 0
  
  VCCT = (1 + 60k/39k + 60k/30k)*FB - 60k/39k*SETVCCT
       ~ 4.5*FB - 1.5*SETVCCT
  SETVCCT = (4.5*FB - VCCT)/1.5;

  max(VCCT) = 4.5*FB ~ 5.5V
  min(VCCT) = 4.5*FB - 1.5*2.5 ~ 1.7V

  DAC_LSB = 2.5V/4096 = 610uV

  DAC_CODE=SETVCCT/DAC_LSB
          =(5.5V-VCCT)/1.5/610uV

	  =(5.5V-VCCT)/915uV

	  = 1000*(5500-VCCT/mV)/915
*/

#define MAX_VCCT       5500
#define MIN_VCCT       1753
#define SETVCCT_LSB_uV  610

static void setup_dac()
{
  DAC12_0DAT = 4095;
  DAC12_0CTL = (DAC12CALON|DAC12IR|DAC12AMP_5); /* Internal ref gain 1 */
}

void dac(int code)
{
  DAC12_0CTL |= DAC12ENC;               
  DAC12_0DAT = code;
  msleep(200);
}

/* --------------------------------------------------------------- */

static void setup_io_ports()
{
  P1OUT = 0;  P2OUT = 0;  P3OUT = 0;
  P4OUT = 0;  P5OUT = 0;  P6OUT = 0;

  /* 
     Initial state on reset or power up.
     PxOUT : unchanged 
     PxDIR : reset with Power-UP Clear
     PxSEL : reset with PUC.
  */
  /*
    P1:   LED and handshake
    P1.0: O   MODE LED 
    P1.1: O   POWER LED
    P1.2:     N.C.
    P1.3:     N.C.
    P1.4: I   URTS (TXD)
    P1.5: O   UDSR
    P1.6: O   UCTS
    P1.7: I   MU2/UDTR
  */
  P1OUT = (BIT6|BIT5);
  P1DIR = (BIT6|BIT5|BIT1|BIT0);

  /*
    P2:  target control
    P2.0:     MU0
    P2.1:     MU1
    P2.2:     RXD
    P2.3:     MU3
    P2.4:     TDI
    P2.5:  O  SELT# 
    P2.6:  O  TGTRST
    P2.7:     SETVF
  */
  P2OUT = (BIT6|BIT5);
  P2DIR = (BIT6|BIT5);

  /* 
    P3:  com port
    P3.0:     N.C.
    P3.1:     SDA (EEPROM, Pulled up by 1.5K)
    P3.2:     N.C.
    P3.3:     SCL (EEPROM, Pulled up by 1.5K)
    P3.4:  O  TXD (To TUSB3410)
    P3.5:  I  RXD (From TUSB3410)
    P3.6:     RST_3410 (Pulled up by 10K)
    P3.7:     N.C.
  */
  P3SEL = (BIT5|BIT4|BIT3|BIT1);
  P3DIR = (BIT4|BIT3|BIT1);

  /*
    P4:    test control
    P4.0: O   TEST#
    P4.1:     TEST#
    P4.2: O   ENI2O# (ENTDI2TDO)
    P4.3: O   VCCTON
    P4.4: O   TDIOFF#
    P4.5: O   VF2TDI
    P4.6: O   VF2TEST
    P4.7:     TDI
  */
  P4OUT = (BIT2|BIT0);
  P4DIR = (BIT6|BIT5|BIT4|BIT3|BIT2|BIT0);

  /*
    P5:   JTAG
    P5.0: O   TMS
    P5.1: O   TDI
    P5.2: I   TDO
    P5.3: O   TCK
    P5.4:     N.C.
    P5.5:     N.C.
    P5.6:     N.C.
    P5.7:     N.C.
  */
  P5DIR = (BIT3|BIT1|BIT0);

  /* P6: Analog
     P6.0:    ADC0 (VCCI sense)
     P6.1:    ADC1 (VCCT sense)
     P6.2:    ADC2 (VF sense)
     P6.3:    ADC3 (VCCR sense)
     P6.4:    N.C.
     P6.5:    N.C.
     P6.6:    DAC0 (SETVCCT)
     P6.7:    N.C.
   */
  P6SEL = (BIT6|BIT3|BIT2|BIT1|BIT0);
}

/* --------------------------------------------------------- */

enum {
  VCCI_POWERED,
  VCCI_NO_POWER,
  VCCI_LOW_POWER,
  VCCI_HIGH_POWER
};

#define vcci()  vcc( 0, VCC_LSB_uV)  /* measure VCCI and return in mV */
#define vcct()  vcc( 1, VCC_LSB_uV)
#define vccr()  vcc( 3, VCC_LSB_uV)
#define vf()    vcc( 2, VF_LSB_uV)
#define vtemp() vcc(10, ADC_LSB_uV)  /* temp sensor */
#define vcch()  vcc(11, ADC_LSB_uV)  /* half of VCC (from on-chip R divider) */

int vcc(int n, int LSB_uV)
{
  int mV;

  mV = ((long)adc(n)*LSB_uV)/1000L;
  return mV;
}

int ivcct()
{
  int uA;
  int t, r;

  r = adc(3); /* VCCR */
  t = adc(1); /* VCCT */
  
  uA=((long)(r-t)*VCC_LSB_uV)*3/2; /* R=1.5 Ohm */

  return uA;
}

int check_vcci()
{
  int v = vcci();

  if (v < 0)
    panic();
  else if (v < 1000)
    return VCCI_NO_POWER;
  else if (v < 1800)
    return VCCI_LOW_POWER;
  else if (v < 3600)
    return VCCI_POWERED;
  else
    return VCCI_HIGH_POWER;
}

int tempsensor_mV_to_K(int mV)
{
  long K;
  /* 
     User's Guide 17-16.

     VTEMP = 0.00355*(TEMP_degC) + 0.986

     TEMP_K = (VTEMP-0.986)/0.00355 + 273
            ~ (mV-986)/3.55 + 273
	    ~ mV/3.55-986/3.55 + 273
	    ~ mV/3.55 - 5
  */
  K = (long)mV*100/355-5;
  return K;
}

/* ---------------------------------------------------- */

void uif_attach_jtag()
{
  set_SELT();
  set_TDION();        
  clr_ENI2O();
  msleep(10);

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
  clr_SELT();
  clr_TDION();        
}

void uif_attach_sbw()
{
  clr_TEST();
  set_SELT();
  clr_TDION();        
  set_ENI2O();
  msleep(10);
  
  /* JTAG - Spy-Bi-Wire entry sequence (8.3.1 of slau265e) */

  set_TDI();
  set_TCK();
  msleep(10);
  clr_TCK();
  set_TCK();

  /*
  clr_TCK();
  set_TCK();
  clr_TCK();
  set_TCK();
  */
}

void uif_detach_sbw()
{
  clr_SELT();
  clr_ENI2O();
  clr_TDION();        
}

void uif_attach_cc8051()
{
  set_SELT();
  set_TDION();        
  set_ENI2O();

  clr_TCK();
  set_TGTRST();
  clr_TGTRST();
  msleep(1);
  set_TCK();    /* first rising edge */
  clr_TCK();
  set_TCK();    /* second rising edge */
  clr_TCK();
  nop();
  set_TGTRST();
}

int uif_set_vcct(int mV)
{
  long    code;

  if ((check_vcci() != VCCI_NO_POWER)||(mV < MIN_VCCT))
    clr_VCCTON();
  else
    set_VCCTON();

  if (mV <  MIN_VCCT) mV = MIN_VCCT;
  if (mV >  MAX_VCCT) mV = MAX_VCCT;
  code = ((long)1000*(5500 - mV))/915; /* see above */
  dac(code);

  return vcct();
}

void uif_reset_target()
{
  /* need SELT */
  clr_TGTRST();
  msleep(10);
  set_TGTRST();
}

void uif_assert_RST_3410()
{
  P3OUT &= ~BIT6;
  P3DIR |= BIT6;
}

void uif_negate_RST_3410()
{
  P3OUT |=  BIT6; /* Just in case */
  P3DIR &= ~BIT6;
}

/* ------------------------------------------------------------------------- */

#define EEPROM_ADDRESS           0x50           // Address of EEPROM
#define OWN_ADDRESS              0x40           // I2C own address

void uif_usart_I2C()
{
  dint();
  U0CTL = SWRST;                                // USART logic in reset state
  U0CTL &= ~I2CEN;                              // Clear I2CEN bit
  {
    U0CTL   = (I2C|SYNC|MST);                   // I2C master mode, 7-bit addr
    I2CTCTL = (I2CTRX|I2CSSEL_ACLK);            // Byte mode, repeat mode, ACLK
    I2CSA   = EEPROM_ADDRESS;                   // I2C slave address
    I2COA   = OWN_ADDRESS;                      // Own address
    I2CPSC  = 3;                                // I2C prd = 4 * clock prd
    I2CSCLH = 8;                                // SCL high prd = 10 * I2C prd
    I2CSCLL = 8;                                // SCL low prd = 10 * I2C prd
  }
  U0CTL |= I2CEN;                               // Enable I2C module operation
  eint();
}

void uif_usart_com()
{
#define BAUDMOD      0x52
  dint();
  U0CTL  = 0;
  U0CTL  = SWRST;       /* Softwawre reset */ 
  {
    ME1   |= (UTXE0|URXE0);
    U0CTL |= CHAR;
    U0TCTL = SSEL_ACLK;
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
  setup_system_clock();
  setup_io_ports();
  setup_adc();
  setup_dac();
  uif_usart_com();
}

interrupt (TIMERA0_VECTOR)  ta_ccr0_isr(){}
interrupt (TIMERB0_VECTOR)  tb_ccr0_isr(){}

/* EOF */
