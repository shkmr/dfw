#ifndef UIF_MACROS_H
#define UIF_MACROS_H

#include <io.h>

#define MCLK                    12000000
#define TUSB3410_BAUDRATE         460800

#define power_LED_on()
#define power_LED_off()  
#define power_LED_flip() 
#define mode_LED_on()    
#define mode_LED_off()
#define mode_LED_flip()  

#define set_TMS()          (P5OUT|=BIT0)
#define clr_TMS()          (P5OUT&=~BIT0)
#define tst_TMS()          (P5OUT&BIT0)

#define set_ENI2O()        (P5DIR|=BIT1)
#define clr_ENI2O()        (P5DIR&=~BIT1)
#define set_TDI()          (P5OUT|=BIT1)
#define clr_TDI()          (P5OUT&=~BIT1)
#define tst_TDI()          (P5OUT&BIT1)

#define tst_TDO()          (P5IN&BIT2)

#define set_TCK()          (P5OUT|=BIT3)
#define clr_TCK()          (P5OUT&=~BIT3)
#define tst_TCK()          (P5OUT&BIT3)

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
