#ifndef TAP_JTAG_H
#define TAP_JTAG_H
#include <stdint.h>

void      jtag_SET_TCLK(void);
void      jtag_CLR_TCLK(void);
uint16_t  jtag_DR_SHIFT16(uint16_t data);
uint32_t  jtag_DR_SHIFT20(uint32_t data);
uint16_t  jtag_IR_SHIFT(uint16_t data);
void      jtag_RESET_TAP(void);
uint16_t  jtag_ATTACH_TAP(void);
void      jtag_DETACH_TAP(void);
void      jtag_TCLK_STROBES();

#endif /* TAP_JTAG_H */



