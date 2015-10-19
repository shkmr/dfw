/* 
 *     PTFW : serial pass-through
 *
 *                        written by skimu@mac.com
 */
#include <stdint.h>
#include <stdio.h>
#include <io.h>
#include <signal.h>

#include "uif.h"
#include "uif_macros.h"
#include "eeprom.h"
#include "ptlink.h"

int  main(void)         __attribute__ ((noreturn));

void init_usart1()
{
  P3SEL |= (BIT7|BIT6); /* BRXD, BTXD */
  P3DIR |= (BIT6);      /* BTXD  */

#define BAUDMOD      0x00
  U1CTL  = 0;
  U1CTL  = SWRST;          /* Softwawre reset */ 
  {
    ME2   |= (UTXE1|URXE1);
    U1CTL |= CHAR;
    U1TCTL = SSEL_SMCLK;
    U1BR0  = (MCLK/9600)&0xff;
    U1BR1  = (MCLK/9600)>>8;
    U1MCTL = BAUDMOD;
  }
  U1CTL &= ~SWRST;                 
  U1IE   = URXIE1;
#undef BAUDMOD
}

extern int nc1, nc0;

int main()
{
  WDTCTL = (WDTPW|WDTHOLD);
  init_ptlink();
  init_uif();
  set_TDI();
  eeprom_firmware_check();
  init_usart1();
  eint();
  while (1) {
    if (nc0 > 0 && (U1IFG&UTXIFG1))  U1TXBUF = getchar0();
    if (nc1 > 0 && (U0IFG&UTXIFG0))  U0TXBUF = getchar1();
  }
}

/* EOF */
