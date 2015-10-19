#ifndef TARGET_MSP430_H
#define TARGET_MSP430_H

void     msp430_set_FCTL3(uint16_t x);
void     msp430_set_FastFlash(void);
void     msp430_set_SlowFlash(void);
int      msp430_device_id(void);

uint16_t msp430_cntrl_sig(void);      /* returns JTAG CNTRL_SIG register  */
int      msp430_is_cpu_stopped(void); /* 1 : stopped, 0 : unknown/running */
uint32_t msp430_stop_cpu(void);       /* stop cpu at instr fech state and returns pc */
void     msp430_continue(void);
void     msp430_step_cpu(uint16_t n);

int      msp430_get_device(void);
int      msp430_release_device(uint32_t addr);

void     msp430_write_registers(uint32_t *regs);
void     msp430_read_registers(uint32_t  *regs);
void     msp430_write_register(int n, uint32_t val);
uint32_t msp430_read_register(int n);

void     msp430_write_mem(int nbits, uint32_t addr, uint16_t data);
void     msp430_write_mem_quick(uint32_t addr, const uint16_t *buf, uint16_t len);
void     msp430_write_flash(uint32_t addr, const uint16_t *buf, uint16_t len);
uint16_t msp430_read_mem(int nbits, uint32_t addr);
void     msp430_read_mem_quick(uint32_t addr, uint16_t *buf, uint16_t len);

#define msp430_ERASE_GLOB    0xA50E // main & info of ALL      mem arrays
#define msp430_ERASE_ALLMAIN 0xA50C // main        of ALL      mem arrays
#define msp430_ERASE_MASS    0xA506 // main & info of SELECTED mem arrays
#define msp430_ERASE_MAIN    0xA504 // main        of SELECTED mem arrays
#define msp430_ERASE_SGMT    0xA502 // SELECTED segment

void    msp430_erase_flash(int mode, uint32_t addr);
int     msp430_is_fuse_blown(void);

#endif /* TARGET_MSP430_H */
