#ifndef TAP_H
#define TAP_H
#include <stdint.h>
#include "tap_jtag.h"
#include "tap_sbw.h"

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN void      (*SET_TCLK)(void);
EXTERN void      (*CLR_TCLK)(void);
EXTERN uint16_t  (*DR_SHIFT16)(uint16_t);
EXTERN uint32_t  (*DR_SHIFT20)(uint32_t);
EXTERN uint16_t  (*IR_SHIFT)(uint16_t);
EXTERN void      (*RESET_TAP)(void);
EXTERN uint16_t  (*ATTACH_TAP)(void);
EXTERN void      (*DETACH_TAP)(void);
EXTERN void      (*TCLK_STROBES)(int);

void tap_set_jtag(void);
void tap_set_sbw(void);

#endif
