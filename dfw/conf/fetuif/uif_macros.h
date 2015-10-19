#ifndef UIF_MACROS_H
#define UIF_MACROS_H

#include <io.h>

/* FET430UIF hardware definitions */

#define MCLK               8000000        /* CPU frequency, 8MHz */

#define TUSB3410_BAUDRATE  460800

#define power_LED_on()      (P1OUT|=BIT1)
#define power_LED_off()     (P1OUT&=~BIT1)
#define power_LED_flip()    (P1OUT^=BIT1)
#define mode_LED_on()       (P1OUT|=BIT0)
#define mode_LED_off()      (P1OUT&=~BIT0)
#define mode_LED_flip()     (P1OUT^=BIT0)

#define set_ENI2O_()        (P4OUT|=BIT2)
#define clr_ENI2O_()        (P4OUT&=~BIT2)
#define tst_ENI2O_()        (P4OUT&BIT2)
#define clr_ENI2O()         set_ENI2O_()
#define set_ENI2O()         clr_ENI2O_()
#define tst_ENI2O()         (!tst_ENI2O_())

#define set_TMS()           (P5OUT|=BIT0)
#define clr_TMS()           (P5OUT&=~BIT0)
#define tst_TMS()           (P5OUT&BIT0)

#define set_TDI()           (P5OUT|=BIT1)
#define clr_TDI()           (P5OUT&=~BIT1)
#define tst_TDI()           (P5OUT&BIT1)

#define set_TCK()           (P5OUT|=BIT3)
#define clr_TCK()           (P5OUT&=~BIT3)
#define tst_TCK()           (P5OUT&BIT3)

#define tst_TDO()           (P5IN&BIT2)

#define TCLK_STROBES_NOPS() {nop();}

#endif /* UIF_MACROS_H */
