#ifndef TARGET_H
#define TARGET_H
#include <stdint.h>

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN int      (*target_get_device)(void);
EXTERN int      (*target_release_device)(uint32_t addr);
EXTERN int      (*target_device_id)(void);
EXTERN void     (*target_set_FCTL3)(uint16_t val);

EXTERN int      (*target_is_cpu_stopped)(void);
EXTERN uint32_t (*target_stop_cpu)(void);
EXTERN void     (*target_continue)(void);
EXTERN void     (*target_step_cpu)(uint16_t);
EXTERN uint16_t (*target_cntrl_sig)(void);

EXTERN void     (*target_write_registers)(uint32_t *regs);
EXTERN void     (*target_read_registers)(uint32_t *regs);
EXTERN void     (*target_write_register)(int n, uint32_t val);

EXTERN void     (*target_write_mem)(int nbits, uint32_t addr, uint16_t data);
EXTERN void     (*target_write_mem_quick)(uint32_t addr, const uint16_t *buf, uint16_t len);
EXTERN void     (*target_write_flash)(uint32_t addr, const uint16_t *buf, uint16_t len);
EXTERN uint16_t (*target_read_mem)(int nbits, uint32_t addr);
EXTERN void     (*target_read_mem_quick)(uint32_t addr, uint16_t *buf, uint16_t len);

EXTERN uint16_t ERASE_GLOB;
EXTERN uint16_t ERASE_ALLMAIN;
EXTERN uint16_t ERASE_MASS;
EXTERN uint16_t ERASE_MAIN;
EXTERN uint16_t ERASE_SGMT;

EXTERN void     (*target_erase_flash)(int mode, uint32_t addr);
EXTERN int      (*target_is_fuse_blown)(void);

void target_set_msp430(void);
void target_set_cc8051(void);
void target_set_cpuX(void);
void target_set_cpuXv2(void);

#endif /* TARGET_H */
