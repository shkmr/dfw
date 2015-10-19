#define EXTERN
#include "target.h"
#undef  EXTERN
#include "target_msp430.h"
#include "target_cc8051.h"
#include "target_cpuX.h"
#include "target_cpuXv2.h"

void target_set_msp430()
{
  target_get_device      = msp430_get_device;
  target_release_device  = msp430_release_device;
  target_device_id       = msp430_device_id;
  target_set_FCTL3       = msp430_set_FCTL3;

  target_is_cpu_stopped  = msp430_is_cpu_stopped;
  target_stop_cpu        = msp430_stop_cpu;
  target_continue        = msp430_continue;
  target_step_cpu        = msp430_step_cpu;
  target_cntrl_sig       = msp430_cntrl_sig;

  target_write_registers = msp430_write_registers;
  target_read_registers  = msp430_read_registers;
  target_write_register  = msp430_write_register;

  target_write_mem       = msp430_write_mem;
  target_write_mem_quick = msp430_write_mem_quick;
  target_write_flash     = msp430_write_flash;
  target_read_mem        = msp430_read_mem;
  target_read_mem_quick  = msp430_read_mem_quick;

  ERASE_GLOB             = msp430_ERASE_GLOB;
  ERASE_ALLMAIN          = msp430_ERASE_ALLMAIN;
  ERASE_MASS             = msp430_ERASE_MASS;
  ERASE_MAIN             = msp430_ERASE_MAIN;
  ERASE_SGMT             = msp430_ERASE_SGMT;

  target_erase_flash     = msp430_erase_flash;
  target_is_fuse_blown   = msp430_is_fuse_blown;
}

void target_set_cc8051()
{
  target_get_device      = cc8051_get_device;
  target_release_device  = cc8051_release_device;
  target_device_id       = cc8051_device_id;
  target_set_FCTL3       = cc8051_set_FCTL3;

  target_is_cpu_stopped  = cc8051_is_cpu_stopped;
  target_stop_cpu        = cc8051_stop_cpu;
  target_continue        = cc8051_continue;
  target_step_cpu        = cc8051_step_cpu;
  target_cntrl_sig       = cc8051_cntrl_sig;

  target_write_registers = cc8051_write_registers;
  target_read_registers  = cc8051_read_registers;
  target_write_register  = cc8051_write_register;

  target_write_mem       = cc8051_write_mem;
  target_write_mem_quick = cc8051_write_mem_quick;
  target_write_flash     = cc8051_write_flash;
  target_read_mem        = cc8051_read_mem;
  target_read_mem_quick  = cc8051_read_mem_quick;

  ERASE_GLOB             = cc8051_ERASE_GLOB;
  ERASE_ALLMAIN          = cc8051_ERASE_ALLMAIN;
  ERASE_MASS             = cc8051_ERASE_MASS;
  ERASE_MAIN             = cc8051_ERASE_MAIN;
  ERASE_SGMT             = cc8051_ERASE_SGMT;

  target_erase_flash     = cc8051_erase_flash;
  target_is_fuse_blown   = cc8051_is_fuse_blown;
}

void target_set_cpuX()
{
  target_get_device      = cpuX_get_device;
  target_release_device  = cpuX_release_device;
  target_device_id       = cpuX_device_id;
  target_set_FCTL3       = cpuX_set_FCTL3;

  target_is_cpu_stopped  = cpuX_is_cpu_stopped;
  target_stop_cpu        = cpuX_stop_cpu;
  target_continue        = cpuX_continue;
  target_step_cpu        = cpuX_step_cpu;
  target_cntrl_sig       = cpuX_cntrl_sig;

  target_write_registers = cpuX_write_registers;
  target_read_registers  = cpuX_read_registers;
  target_write_register  = cpuX_write_register;

  target_write_mem       = cpuX_write_mem;
  target_write_mem_quick = cpuX_write_mem_quick;
  target_write_flash     = cpuX_write_flash;
  target_read_mem        = cpuX_read_mem;
  target_read_mem_quick  = cpuX_read_mem_quick;

  ERASE_GLOB             = cpuX_ERASE_GLOB;
  ERASE_ALLMAIN          = cpuX_ERASE_ALLMAIN;
  ERASE_MASS             = cpuX_ERASE_MASS;
  ERASE_MAIN             = cpuX_ERASE_MAIN;
  ERASE_SGMT             = cpuX_ERASE_SGMT;

  target_erase_flash     = cpuX_erase_flash;
  target_is_fuse_blown   = cpuX_is_fuse_blown;
}

void target_set_cpuXv2()
{
  target_get_device      = cpuXv2_get_device;
  target_release_device  = cpuXv2_release_device;
  target_device_id       = cpuXv2_device_id;
  target_set_FCTL3       = cpuXv2_set_FCTL3;

  target_is_cpu_stopped  = cpuXv2_is_cpu_stopped;
  target_stop_cpu        = cpuXv2_stop_cpu;
  target_continue        = cpuXv2_continue;
  target_step_cpu        = cpuXv2_step_cpu;
  target_cntrl_sig       = cpuXv2_cntrl_sig;

  target_write_registers = cpuXv2_write_registers;
  target_read_registers  = cpuXv2_read_registers;
  target_write_register  = cpuXv2_write_register;

  target_write_mem       = cpuXv2_write_mem;
  target_write_mem_quick = cpuXv2_write_mem_quick;
  target_write_flash     = cpuXv2_write_flash;
  target_read_mem        = cpuXv2_read_mem;
  target_read_mem_quick  = cpuXv2_read_mem_quick;

  ERASE_GLOB             = cpuXv2_ERASE_GLOB;
  ERASE_ALLMAIN          = cpuXv2_ERASE_ALLMAIN;
  ERASE_MASS             = cpuXv2_ERASE_MASS;
  ERASE_MAIN             = cpuXv2_ERASE_MAIN;
  ERASE_SGMT             = cpuXv2_ERASE_SGMT;

  target_erase_flash     = cpuXv2_erase_flash;

  target_is_fuse_blown   = cpuXv2_is_fuse_blown;
}

#if 0
void cpuXv2_set_265e()
{
  target_write_flash = cpuXv2_write_flash_265e;
  target_erase_flash = cpuXv2_erase_flash_265e;
}

void cpuXv2_set_265e_wo_release()
{
  target_write_flash = cpuXv2_write_flash_265e_wo_release;
  target_erase_flash = cpuXv2_erase_flash_265e_wo_release;
}
#endif
