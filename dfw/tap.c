#define EXTERN
#include "tap.h"
#undef  EXTERN

void tap_set_jtag()
{
  SET_TCLK     = jtag_SET_TCLK;
  CLR_TCLK     = jtag_CLR_TCLK;
  DR_SHIFT16   = jtag_DR_SHIFT16;
  DR_SHIFT20   = jtag_DR_SHIFT20;
  IR_SHIFT     = jtag_IR_SHIFT;
  RESET_TAP    = jtag_RESET_TAP;
  ATTACH_TAP   = jtag_ATTACH_TAP;
  DETACH_TAP   = jtag_DETACH_TAP;
  TCLK_STROBES = jtag_TCLK_STROBES;
}

void tap_set_sbw()
{
  SET_TCLK     = sbw_SET_TCLK;
  CLR_TCLK     = sbw_CLR_TCLK;
  DR_SHIFT16   = sbw_DR_SHIFT16;
  DR_SHIFT20   = sbw_DR_SHIFT20;
  IR_SHIFT     = sbw_IR_SHIFT;
  RESET_TAP    = sbw_RESET_TAP;
  ATTACH_TAP   = sbw_ATTACH_TAP;
  DETACH_TAP   = sbw_DETACH_TAP;
  TCLK_STROBES = sbw_TCLK_STROBES;
}
