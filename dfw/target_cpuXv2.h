#ifndef TARGET_CPUXV2_H
#define TARGET_CPUXV2_H
void     cpuXv2_set_FastFlash(void);
int      cpuXv2_coreip_id(void);
int      cpuXv2_device_id(void);
uint32_t cpuXv2_device_id_pointer(void);
void     cpuXv2_set_FCTL_base(uint16_t address);
void     cpuXv2_set_FCTL3(uint16_t val);

uint16_t cpuXv2_cntrl_sig(void);      /* returns JTAG CNTRL_SIG register  */
int      cpuXv2_is_cpu_stopped(void); /* 1 : stopped, 0 : unknown/running */
uint32_t cpuXv2_stop_cpu(void);       /* stop cpu at instr fech state and returns pc */
void     cpuXv2_continue(void);
void     cpuXv2_step_cpu(uint16_t n);

int      cpuXv2_get_device(void);
int      cpuXv2_release_device(uint32_t addr);

void     cpuXv2_write_registers(uint32_t *regs);
void     cpuXv2_read_registers(uint32_t  *regs);
void     cpuXv2_write_register(int n, uint32_t val);
uint32_t cpuXv2_read_register(int n);

void     cpuXv2_write_mem(int nbits, uint32_t addr, uint16_t data);
void     cpuXv2_write_mem_quick(uint32_t addr, const uint16_t *buf, uint16_t len);
void     cpuXv2_write_flash(uint32_t addr, const uint16_t *buf, uint16_t len);
void     cpuXv2_write_flash_265e(uint32_t addr, const uint16_t *buf, uint16_t len);
void     cpuXv2_write_flash_265e_wo_release(uint32_t addr, const uint16_t *buf, uint16_t len);
uint16_t cpuXv2_read_mem(int nbits, uint32_t addr);
void     cpuXv2_read_mem_quick(uint32_t addr, uint16_t *buf, uint16_t len);

#define cpuXv2_ERASE_GLOB    0xA506
#define cpuXv2_ERASE_ALLMAIN 0xA506
#define cpuXv2_ERASE_MASS    0xA506 // mass
#define cpuXv2_ERASE_MAIN    0xA504 // bank
#define cpuXv2_ERASE_SGMT    0xA502 // segment

void    cpuXv2_erase_flash(int mode, uint32_t addr);
void    cpuXv2_erase_flash_265e(int mode, uint32_t addr);
void    cpuXv2_erase_flash_265e_wo_release(int mode, uint32_t addr);
int     cpuXv2_is_fuse_blown(void);

void    cpuXv2_set_265e(void);               /* in target.c */
void    cpuXv2_set_265e_wo_release(void);    /* in target.c */

#endif /* TARGET_CPUXV2_H */
