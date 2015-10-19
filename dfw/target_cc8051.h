#ifndef TARGET_CC8051_H
#define TARGET_CC8051_H

/* Part.1 : D/FW standard target methods */

int      cc8051_device_id(void);
void     cc8051_set_FCTL3(uint16_t val);
uint16_t cc8051_cntrl_sig(void);
int      cc8051_is_cpu_stopped(void);
uint32_t cc8051_stop_cpu(void);
void     cc8051_continue(void);
void     cc8051_step_cpu(uint16_t n);

int      cc8051_get_device(void);
int      cc8051_release_device(uint32_t addr);

void     cc8051_write_registers(uint32_t *regs);
void     cc8051_read_registers(uint32_t  *regs);
void     cc8051_write_register(int n, uint32_t val);
uint32_t cc8051_read_register(int n);

void     cc8051_write_mem(int nbits, uint32_t addr, uint16_t data);
void     cc8051_write_mem_quick(uint32_t addr, const uint16_t *buf, uint16_t len);
void     cc8051_write_flash(uint32_t addr, const uint16_t *buf, uint16_t len);
uint16_t cc8051_read_mem(int nbits, uint32_t addr);
void     cc8051_read_mem_quick(uint32_t addr, uint16_t *buf, uint16_t len);

enum {
  cc8051_ERASE_GLOB,
  cc8051_ERASE_ALLMAIN,
  cc8051_ERASE_MASS,
  cc8051_ERASE_MAIN,
  cc8051_ERASE_SGMT,
};
void     cc8051_erase_flash(int mode, uint32_t addr);
int      cc8051_is_fuse_blown(void);

/* Part.2 : CC8051 specific methods */

/* Debug configuration (Table 46 of SWRS055F) */

#define TIMERS_OFF          0x08
#define DMA_PAUSE           0x04
#define TIMER_SUSPEND       0x02
#define SEL_FLASH_INFO_PAGE 0x01

void     cc8051_wr_config(uint8_t c);
uint8_t  cc8051_rd_config(void);

/* Debug status (Table 47 of SWRS055F) */

#define CHIP_ERASE_DONE     0x80
#define PCON_IDLE           0x40
#define CPU_HALTED          0x20
#define POWER_MODE_0        0x10
#define HALT_STATUS         0x08
#define DEBUG_LOCKED        0x04
#define OSCILLATOR_STABLE   0x02
#define STACK_OVERFLOW      0x01

uint8_t  cc8051_read_status(void);
uint16_t cc8051_get_pc(void);
void     cc8051_debug_instr1(uint8_t in1, uint8_t *out);
void     cc8051_debug_instr2(uint8_t in1, uint8_t in2, uint8_t *out);
void     cc8051_debug_instr3(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t *out);
void     cc8051_step_replace1(uint8_t in1, uint8_t *out);
void     cc8051_step_replace2(uint8_t in1, uint8_t in2, uint8_t *out);
void     cc8051_step_replace3(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t *out);
void     cc8051_read_code(uint16_t addr, uint8_t bank, uint8_t *buf, uint16_t len);
void     cc8051_read_xdata(uint16_t addr, uint8_t *buf, uint16_t len);
void     cc8051_write_xdata(uint16_t addr, const uint8_t *buf, uint16_t len);
void     cc8051_set_pc(uint16_t addr);
void     cc8051_set_FLASH_WORD_SIZE(int x);
void     cc8051_write_flash_page(uint32_t laddr, const uint8_t *buf, uint16_t len);
void     cc8051_mass_erase_flash(void);

#endif /* TARGET_CC8051_H */
