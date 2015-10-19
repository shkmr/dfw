/*
  LED flasher to check GDB's stop and continue

 */
#include <io.h>

#define MCLK    1000000             /* DCO clock, 1MHz */
#define SPEED   10                  /* smaller faster  */

#define led_on()  {P1OUT |=  BIT0;}
#define led_off() {P1OUT &= ~BIT0;}

void msleep(int n)
{
  volatile int i;
  while (n-- > 0)
    for (i=MCLK/6000; i > 0; i--) ;
}

void led_message(char *s)
{
  int c;
  led_off();
  msleep(70*SPEED);
  while (c=*s++) {
    switch (c) {
    case '.':
      led_on();
      msleep(10*SPEED);
      led_off();
      msleep(10*SPEED);
      break;
    case '-':
    case '_':
      led_on();
      msleep(25*SPEED);
      led_off();
      msleep(10*SPEED);
      break;
    case ' ':
      msleep(30*SPEED);
      break;
    default: ;
    }
  }
  msleep(70*SPEED);
}

int main()
{ 
  WDTCTL = (WDTPW|WDTHOLD);
  P1DIR |= 0x01;

#ifdef CALBC1_1MHZ_
  if (CALDCO_1MHZ != 0xff &&
      CALBC1_1MHZ != 0xff) {
    DCOCTL  = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ;
  }
#endif

  while (1) {
    led_message("-.-.-  .- -... -.-. -.. .  ");
    LPM0;
    led_message("-.. -... --.");
    LPM0;
  }
}

/* EOF */
