#ifndef UIF_MACROS_H
#define UIF_MACROS_H

#include <io.h>

#define MCLK                    12000000
#define USCI_BAUDRATE             460800

/* LED : RF2500T has two LEDs */
#define power_LED_on()     (P1OUT|=BIT0)
#define power_LED_off()    (P1OUT&=~BIT0)
#define power_LED_flip()   (P1OUT^=BIT0)
#define mode_LED_on()      (P1OUT|=BIT1)
#define mode_LED_off()     (P1OUT&=~BIT1)
#define mode_LED_flip()    (P1OUT^=BIT1)

/* JTAG : Needs to be consistent with uif_f2274.c */
#define set_ENI2O()        (P2DIR|=BIT4)
#define clr_ENI2O()        (P2DIR&=~BIT4)
#define set_TDI()          (P2OUT|=BIT4)
#define clr_TDI()          (P2OUT&=~BIT4)
#define tst_TDI()          (P2OUT&BIT4)

#define tst_TDO()          (P2IN&BIT2)

#define set_TMS()          (P4OUT|=BIT4)
#define clr_TMS()          (P4OUT&=~BIT4)
#define tst_TMS()          (P4OUT&BIT4)

#define set_TCK()          (P4OUT|=BIT6)
#define clr_TCK()          (P4OUT&=~BIT6)
#define tst_TCK()          (P4OUT&BIT6)

/* TCLK_STROBES_NOPS:
 * 
 *     Used in tap_jtag.c and tap_sbw.c to generate flash
 *     timing clocks.  Need to be adjusted for MCLK setting.
 *     Here we have 10 nops for 12MHz.
 */
#define TCLK_STROBES_NOPS() \
  {                         \
    nop();                  \
    nop();                  \
    nop();                  \
    nop();                  \
    nop();                  \
    nop();                  \
    nop();                  \
    nop();                  \
    nop();                  \
    nop();                  \
  }

#endif /* UIF_MACROS_H */
