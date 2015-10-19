#ifndef TARGET_CPUX_H
#define TARGET_CPUX_H
void     cpuX_set_FastFlash(void);
int      cpuX_device_id(void);
void     cpuX_set_FCTL3(uint16_t val);

uint16_t cpuX_cntrl_sig(void);      /* returns JTAG CNTRL_SIG register  */
int      cpuX_is_cpu_stopped(void); /* 1 : stopped, 0 : unknown/running */
uint32_t cpuX_stop_cpu(void);       /* stop cpu at instr fech state and returns pc */
void     cpuX_continue(void);
void     cpuX_step_cpu(uint16_t n);

int      cpuX_get_device(void);
int      cpuX_release_device(uint32_t addr);

void     cpuX_write_registers(uint32_t *regs);
void     cpuX_read_registers(uint32_t  *regs);
void     cpuX_write_register(int n, uint32_t val);
uint32_t cpuX_read_register(int n);

void     cpuX_write_mem(int nbits, uint32_t addr, uint16_t data);
void     cpuX_write_mem_quick(uint32_t addr, const uint16_t *buf, uint16_t len);
void     cpuX_write_flash(uint32_t addr, const uint16_t *buf, uint16_t len);
uint16_t cpuX_read_mem(int nbits, uint32_t addr);
void     cpuX_read_mem_quick(uint32_t addr, uint16_t *buf, uint16_t len);

#define cpuX_ERASE_GLOB    0xA50E // main & info of ALL      mem arrays
#define cpuX_ERASE_ALLMAIN 0xA50C // main        of ALL      mem arrays
#define cpuX_ERASE_MASS    0xA506 // main & info of SELECTED mem arrays
#define cpuX_ERASE_MAIN    0xA504 // main        of SELECTED mem arrays
#define cpuX_ERASE_SGMT    0xA502 // SELECTED segment

void    cpuX_erase_flash(int mode, uint32_t addr);
int     cpuX_is_fuse_blown(void);

#endif /* TARGET_CPUX_H */
