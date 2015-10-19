#ifndef TAP_SBW_H
#define TAP_SBW_H
#include <stdint.h>

void     sbw_SET_TCLK(void);
void     sbw_CLR_TCLK(void);
uint16_t sbw_DR_SHIFT16(uint16_t data);
uint32_t sbw_DR_SHIFT20(uint32_t data);
uint16_t sbw_IR_SHIFT(uint16_t data);
void     sbw_RESET_TAP();
uint16_t sbw_ATTACH_TAP();
void     sbw_DETACH_TAP();
void     sbw_TCLK_STROBES();
#endif


