/*
 *    JTAG functions for msp430.
 *
 *    Derived from reference code associated with,
 *    Markus Koesler, Franz Graf, and Zack Albus, 
 *    ``Programming a Flash-Based MSP430 Using JTAG  Interface'', 
 *    Application Report SLAA149A, Texas Instruments, December 2005,
 *    and,
 *    ``MSP430 Memory Programming User's Guide'',  
 *    SLAU265E, Texas Instruments, Revised January 2010.
 *
 *    Please find disclaimer at the end of this file.
 */
#include <stdint.h>

#include "tap.h"
#include "target_msp430.h"
#include "jtag89.h"
#include "misc.h"

static uint16_t  device_id           =  0;
static uint16_t  FCTL3_val           =  0xa500;           /* 0xa540 to unlock INFOA */
static uint16_t  has_FastFlash       =  1;
static uint16_t  reg[16];                                 /* register mirror */
static int       regs_saved          =  0;

void   msp430_set_FCTL3(uint16_t x)  {FCTL3_val = (x&0xff)|0xa500;}
void   msp430_set_FastFlash()        {has_FastFlash = 1;}
void   msp430_set_SlowFlash()        {has_FastFlash = 0;}
int    msp430_device_id()            {return device_id;}

static int is_target_has_FastFlash() {return has_FastFlash;}

/* ----------------------------------------------------------------------- */

uint16_t msp430_cntrl_sig()
{
  IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
  return DR_SHIFT16(0);
}

static inline int cpu_synced()
{
  return msp430_cntrl_sig()&SIG_TCE;
}

static inline int sig_instr_load(uint16_t sig)
{
  return sig&SIG_INSTR_LOAD;
}

static int  cpu_instr_load()
{
  return sig_instr_load(msp430_cntrl_sig());
}

/* ----------------------------------------------------------------------- */

static int set_instr_fetch()
{
  int    i;

  IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
  for (i = 50; i > 0; i--)  {
    if (DR_SHIFT16(0)&SIG_INSTR_LOAD)
      return OK;
    CLR_TCLK();
    SET_TCLK();
  }
  return ERR;
}

static void set_pc(uint32_t addr)
{
  set_instr_fetch();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x3401);
  IR_SHIFT(IR_DATA_16BIT);
  DR_SHIFT16(0x4030);              // "mov #addr, PC" instruction, 3 cycles
  CLR_TCLK();

  SET_TCLK();
  DR_SHIFT16(addr);             
  CLR_TCLK();

  SET_TCLK();
  CLR_TCLK();                      // Now the PC should be on addr

  SET_TCLK();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x2401);              // JTAG has control of RW & BYTE.
}

void msp430_set_pc(uint32_t addr) {set_pc(addr);}

static int write_register(uint16_t val, int n)
{
  uint16_t sig;

  if (n > 15) panic();
  if (n == 3) panic();
  if (n == 0) panic();

  set_instr_fetch();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x3401);              // CPU has control of RW & BYTE.
  IR_SHIFT(IR_DATA_16BIT);
  DR_SHIFT16(0x4030|n);            // "mov @pc+, Rn ==  mov #val, Rn", 2 cycles
  CLR_TCLK();
  SET_TCLK();

  DR_SHIFT16(val);             
  CLR_TCLK();
  SET_TCLK();

  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  sig = DR_SHIFT16(0x2401);        // JTAG has control of RW & BYTE.
  if (!sig_instr_load(sig))      led_message(".-. . --. .-- .-- .--"); 

  return OK;
}

static uint16_t read_register(int n)
{
  uint16_t       val;

  if (n > 15) panic();
  if (n == 0) panic();

  n = (0x4082|((n<<8)&0x0f00)); // "mov Rn, xxxx(R2) ==  mov Rn, &xxxx", 4 cycles

  set_instr_fetch();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x3401);              // CPU has control of RW & BYTE.

  IR_SHIFT(IR_DATA_16BIT);
  DR_SHIFT16(n);            
  CLR_TCLK();
  SET_TCLK();

  DR_SHIFT16(0x00fe);
  CLR_TCLK();
  SET_TCLK();

  if (cpu_instr_load())       led_message(".-. . --. .. .. .. ..");
  CLR_TCLK();
  SET_TCLK();

  if (cpu_instr_load())       led_message(".-. . --. ... ... ...");
  CLR_TCLK();                 
  IR_SHIFT(IR_DATA_CAPTURE);
  val = DR_SHIFT16(0);
  SET_TCLK();

  if (!cpu_instr_load())      led_message(".-. . --. .... .... ....");

  return val;
}

/* ----------------------------------------------------------------------- */

static void save_regs()
{
  int    n;

  for (n = 15; n >= 4; n--)
    reg[n] = read_register(n);
  reg[3] = 0;                       /* constant generator */
  reg[2] = read_register(2);        /* SR */
  reg[1] = read_register(1);        /* SP */
}

static void restore_regs()
{
  int    n;

  for (n = 15; n >= 4; n--)
    write_register(reg[n], n);
  write_register(reg[2], 2);
  write_register(reg[1], 1);
}

uint32_t msp430_stop_cpu()
{
  void enter_emulation_mode() {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT16(0x3401);
    IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
    while (!(DR_SHIFT16(0)&SIG_TCE1)) {
      CLR_TCLK();
      SET_TCLK();
    }
    CLR_TCLK();
    SET_TCLK();
  };

  void set_instr_fetch() {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT16(0x2401);
    IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
    while (!(DR_SHIFT16(0)&SIG_INSTR_LOAD)) {
      CLR_TCLK();
      SET_TCLK();
    }
  }

  enter_emulation_mode();
  set_instr_fetch();

  IR_SHIFT(IR_ADDR_CAPTURE); 
  reg[0] = DR_SHIFT16(0);
  save_regs();
  regs_saved = 1;
  /* wake CPU up for late memory operation  */
  write_register(0, 2); // set SR = 0

  return reg[0];
}

int msp430_is_cpu_stopped() 
{
  if (!regs_saved && (msp430_cntrl_sig()&SIG_CPUOFF))
    msp430_stop_cpu();
  return regs_saved;
} 

void msp430_continue()
{
  restore_regs();
  msp430_release_device(0);
}

void msp430_step_cpu(uint16_t n)
{
  if (reg[2]&0x0010)  return;

  restore_regs();
  set_pc(reg[0]);

  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x3401);
  IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
  while (n > 0) {
    CLR_TCLK();
    SET_TCLK();
    if (DR_SHIFT16(0)&SIG_INSTR_LOAD) n--;
  }

  IR_SHIFT(IR_ADDR_CAPTURE);
  reg[0] = DR_SHIFT16(0);
  save_regs();
}

void msp430_write_registers(uint32_t *r)
{
  int    n;

  if (!regs_saved) return;

  for (n = 0; n < 16; n++)
    reg[n] = r[n];
  reg[3] = 0; /* constant generator */
}

void msp430_read_registers(uint32_t *r)
{
  int    n;

  if (!regs_saved && cpu_synced()) save_regs();

  for (n = 0; n < 16; n++) r[n] = reg[n];
  r[3] = 0;   /* constant generator */
}

void msp430_write_register(int n, uint32_t val)
{
  if (!regs_saved)        return;
  if (n < 16 && n != 3)   reg[n] = val;
}

/* ----------------------------------------------------------------------- */

static int msp430_execute_POR()
{
  int    JtagVersion;

  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x2C01);         // Apply Reset
  DR_SHIFT16(0x2401);         // Remove Reset
  CLR_TCLK();
  SET_TCLK();
  CLR_TCLK();
  SET_TCLK();
  CLR_TCLK();
  // read JTAG ID, checked at function end
  JtagVersion = IR_SHIFT(IR_ADDR_CAPTURE);
  SET_TCLK();

  save_regs();
  reg[0] = msp430_read_mem(16, 0xfffe);   /* set pc reset vector */
  regs_saved = 1;

  msp430_write_mem(16, 0x0120, 0x5A80);   /* disable Watchdog on target device */
  
  if (JtagVersion != JTAG_ID)
    return ERR;
  return OK;
}

static uint16_t msp430_halt_cpu()
{
  set_instr_fetch();
  IR_SHIFT(IR_DATA_16BIT);
  DR_SHIFT16(0x3FFF);            // Send JMP $ instruction
  CLR_TCLK();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x2409);            // Set JTAG_HALT bit
  SET_TCLK();
  return OK;
}

static int msp430_release_cpu()
{
  CLR_TCLK();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x0401);            // Clear the SUSP bit
  IR_SHIFT(IR_ADDR_CAPTURE);
  SET_TCLK();
  return OK;
}

int msp430_get_device()
{
  int    i;

  regs_saved = 0;
  
  i = 0;
  do {
    if (i++ > 5) {
      led_message(".. -..");
      return ERR;
    }
    DETACH_TAP(); 
    ATTACH_TAP();
    if (msp430_is_fuse_blown())  return ERR;

    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT16(0x2401);      // (JTAGFUNCSAT|TCE1|READ)

  } while (IR_SHIFT(IR_CNTRL_SIG_CAPTURE) != JTAG_ID);
  
  for (i = 0; i < 50; i++) {
    if (DR_SHIFT16(0)&SIG_TCE)
      goto got_device;
  }
  led_message("... -.-- -. -.-.");
  return ERR;

 got_device:
  device_id = msp430_read_mem(16, 0x0FF0);
  device_id = (device_id << 8) + (device_id >> 8);
  
  if (msp430_execute_POR() != OK) {
    led_message(".--. --- .-.");
    return ERR;
  }
  return OK;
}

int msp430_release_device(uint32_t addr)
{
  if (addr == 0xfffe)  {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT16(0x2C01);         // Perform a reset
    DR_SHIFT16(0x2401);
  } else if (addr == 0) {
    set_pc(reg[0]);
  } else {
    set_pc(addr);
  }
  regs_saved = 0;
  IR_SHIFT(IR_CNTRL_SIG_RELEASE);
  return OK;
}

/* ----------------------------------------------------------------------- */
/*
 *    inlining saves a few bytes of stack, increase of ~500 bytes of code.
 */
#define inline_rdwr 
//#define inline_rdwr inline

static inline_rdwr void wr(uint16_t sig, uint32_t addr, uint16_t data)
{
  CLR_TCLK();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(sig);
  IR_SHIFT(IR_ADDR_16BIT);
  DR_SHIFT16(addr);
  IR_SHIFT(IR_DATA_TO_ADDR);
  DR_SHIFT16(data);
  SET_TCLK();
}
static inline_rdwr void wrw(uint32_t addr, uint16_t data) {wr(0x2408, addr, data);}
static inline_rdwr void wrb(uint32_t addr, uint16_t data) {wr(0x2418, addr, data);}

static inline_rdwr uint16_t rd(uint16_t sig, uint32_t addr)
{
  uint16_t     data;

  CLR_TCLK();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(sig); 
  IR_SHIFT(IR_ADDR_16BIT);
  DR_SHIFT16(addr);
  IR_SHIFT(IR_DATA_TO_ADDR);
  SET_TCLK();
  CLR_TCLK();
  data = DR_SHIFT16(0x0000);   // Shift out 16 bits
  // SET_TCLK(); ???
  return data;
}
static inline_rdwr uint16_t rdw(uint32_t addr) {return rd(0x2409, addr);}
static inline_rdwr uint16_t rdb(uint32_t addr) {return rd(0x2419, addr);}

#undef inline_rdwr

/* ----------------------------------------------------------------------- */

void msp430_write_mem(int nbits, uint32_t addr, uint16_t data)
{
  msp430_halt_cpu();
  switch (nbits) {
  case 16: wrw(addr, data);  break;
  case  8: wrb(addr, data);  break;
  default: panic();
  }
  msp430_release_cpu();
}

void msp430_write_mem_quick(uint32_t addr, const uint16_t *buf, uint16_t len)
{
  set_pc(addr-4);
  msp430_halt_cpu();
  CLR_TCLK();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x2408);

  IR_SHIFT(IR_DATA_QUICK);
  while (len-- > 0) {
    DR_SHIFT16(*buf++);
    SET_TCLK();
    CLR_TCLK();
  }
  // SET_TCLK(); ???
  msp430_release_cpu();
}

uint16_t msp430_read_mem(int nbits, uint32_t addr)
{
  uint16_t     data;
  
  msp430_halt_cpu();
  switch (nbits) {
  case 16: data = rdw(addr); break;
  case  8: data = rdb(addr); break;
  default: panic();
  }
  msp430_release_cpu();
  return nbits == 8 ? data & 0x00ff : data;
}

void msp430_read_mem_quick(uint32_t addr, uint16_t *buf, uint16_t len)
{
  int    i;

  set_pc(addr-4);
  msp430_halt_cpu();
  CLR_TCLK();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x2409);
  IR_SHIFT(IR_DATA_QUICK);
  for (i = 0; i < len; i++) {
    SET_TCLK();
    *buf++ = DR_SHIFT16(0x0000);
    CLR_TCLK();
  }
  // SET_TCLK(); ???
  msp430_release_cpu();
}

void msp430_write_flash(uint32_t addr, const uint16_t *buf, uint16_t len)
{
  int        i;

  msp430_halt_cpu();
  wrw(0x0128, 0xa540);                   //   FCTL1 = WRT
  wrw(0x012a, 0xa540);                   //   FCTL2 = FSSEL_MCLK, DIV=1
  wrw(0x012c, FCTL3_val);                //   FCTL3
  for (i = 0; i < len; i++, addr += 2)  {
    wrw(addr, *buf++);
    {
      /*  provide TCLKs.
       *     minimum 33 TCLKs for F149 and F449.
       *     29 is OK for F2xxx.
       */
      CLR_TCLK();
      IR_SHIFT(IR_CNTRL_SIG_16BIT);
      DR_SHIFT16(0x2409);
      TCLK_STROBES(35);
      SET_TCLK();
    }
  }
  wrw(0x0128, 0xa500);                   //   FCTL1
  wrw(0x012c, FCTL3_val);                //   FCTL3, writing the same value will
                                         //   bring LOCKA state back to the original.
  msp430_release_cpu();
}

void msp430_erase_flash(int mode, uint32_t addr)
{
  int       StrobeAmount = 4820;         // default for Segment Erase
  int       i, loopcount = 1;            // erase cycle repeating for Mass Erase

  if ((mode == msp430_ERASE_MASS) || (mode == msp430_ERASE_MAIN)) {
    if (is_target_has_FastFlash()) {
      StrobeAmount = 10600;
    } else {
      StrobeAmount = 5300;               // larger flash require
      loopcount = 19;                    // additional cycles for erase.
    }
  }

  msp430_halt_cpu();
  for (i = loopcount; i > 0; i--) {
    wrw(0x0128, mode);                   // FCTL1 = mode
    wrw(0x012a, 0xa540);                 // FCTL2 = FSSEL_MCLK, DIV=1
    wrw(0x012c, FCTL3_val);
    wrw(addr, 0x55aa);                   // dummy write
    { 
      /*  provide TCLKs.
       *      minimum 33 TCLKs for F149 and F449.
       *      29 is OK for F2xxx.
       */
      CLR_TCLK();
      IR_SHIFT(IR_CNTRL_SIG_16BIT);
      DR_SHIFT16(0x2409);
      TCLK_STROBES(StrobeAmount);
      SET_TCLK();
    }
    wrw(0x0128, 0xa500);                  // FCTL1
  }
  wrw(0x012c, FCTL3_val);                 // FCTL3
  msp430_release_cpu();
}

int msp430_is_fuse_blown()
{
  int    i;
  
  for (i = 3; i > 0; i--) {    //  First trial could be wrong
    IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
    if (DR_SHIFT16(0xAAAA) == 0x5555) {
      return 1;  // Fuse is blown
    }
  }
  return 0;
}

/* slau265d/JTAG Programming/Readme.txt :

THIS SOFTWARE IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY, 
INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS 
FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE. 
TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET 
POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY 
INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR 
YOUR USE OF THE PROGRAM.

IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY 
THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED 
OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT 
OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM. 
EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF 
REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS 
OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF 
USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S 
AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF 
YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS 
(U.S.$500).

Unless otherwise stated, the Program written and copyrighted 
by Texas Instruments is distributed as "freeware".  You may, 
only under TI's copyright in the Program, use and modify the 
Program without any charge or restriction.  You may 
distribute to third parties, provided that you transfer a 
copy of this license to the third party and the third party 
agrees to these terms by its first use of the Program. You 
must reproduce the copyright notice and any other legend of 
ownership on each copy or partial copy, of the Program.

You acknowledge and agree that the Program contains 
copyrighted material, trade secrets and other TI proprietary 
information and is protected by copyright laws, 
international copyright treaties, and trade secret laws, as 
well as other intellectual property laws.  To protect TI's 
rights in the Program, you agree not to decompile, reverse 
engineer, disassemble or otherwise translate any object code 
versions of the Program to a human-readable form.  You agree 
that in no event will you alter, remove or destroy any 
copyright notice included in the Program.  TI reserves all 
rights not specifically granted under this license. Except 
as specifically provided herein, nothing in this agreement 
shall be construed as conferring by implication, estoppel, 
or otherwise, upon you, any license or other right under any 
TI patents, copyrights or trade secrets.
*/
/* EOF */
