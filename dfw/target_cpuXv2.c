/*
 *    JTAG functions for cpuXv2 (MSP430X, 5xx), 
 *
 *    Derived from reference code associated with,
 *    ``MSP430 Memory Programming User's Guide'',  
 *    SLAU265E, Texas Instruments, Revised January 2010.
 *
 *    Please find disclaimer at the end of this file.
 */
#include "tap.h"
#include "target_cpuXv2.h"
#include "jtag91.h"
#include "misc.h"

static uint16_t  coreip_id;
static uint32_t  device_id_pointer;
static uint16_t  device_id;
static uint32_t  reg[16];                                   /* register mirror */
static int       regs_saved       = 0;

static uint16_t  FCTL_base = 0x0140;    /* Flash control register base address */
#define          FCTL1      (FCTL_base+0)
#define          FCTL3      (FCTL_base+4)
#define          FCTL4      (FCTL_base+6)
static uint16_t  FCTL3_val    = 0xa500;

int      cpuXv2_coreip_id()         {return coreip_id; }
int      cpuXv2_device_id()         {return device_id; }
uint32_t cpuXv2_device_id_pointer() {return device_id_pointer;}
void     cpuXv2_set_FCTL_base(uint16_t addr) {FCTL_base = addr;}
void     cpuXv2_set_FCTL3(uint16_t x)        {FCTL3_val = (x&0xff)|0xa500;}

/* ----------------------------------------------------------------------- */

uint16_t cpuXv2_cntrl_sig()
{
  IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
  return DR_SHIFT16(0);
}

static inline int sig_full_emulation(uint16_t sig)
{
  return (sig&(SIG_TCE0|SIG_CPUSUSP)) == (SIG_TCE0|SIG_CPUSUSP);
}

static inline int sig_instr_load(uint16_t sig)
{
  return sig&SIG_INSTR_LOAD;
}

static int  cpu_full_emulation()
{
  return sig_full_emulation(cpuXv2_cntrl_sig());
}

static int  cpu_instr_load()
{
  return sig_instr_load(cpuXv2_cntrl_sig());
}

/* ----------------------------------------------------------------------- */

static void set_pc(uint32_t addr)
{
  if (!cpu_full_emulation()) panic();
    
  CLR_TCLK();
  IR_SHIFT(IR_DATA_16BIT);
  SET_TCLK();
  /* "mova #addr20,PC"  3 cycles */
  DR_SHIFT16(0x0080|((addr>>8)&0x0F00));

  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x1400);
  IR_SHIFT(IR_DATA_16BIT);
  CLR_TCLK();
  SET_TCLK();

  DR_SHIFT16(addr);
  CLR_TCLK();
  SET_TCLK();

  DR_SHIFT16(0x4303);  /* nop (mov R3 R3) */
  CLR_TCLK();
  IR_SHIFT(IR_ADDR_CAPTURE);
  if (DR_SHIFT20(0) != addr) led_message("... . - .--. -.-.");
  SET_TCLK();
}

void cpuXv2_set_pc(uint32_t addr) {set_pc(addr);}

/*
00004478 <main>:
    4478:	31 40 00 44 	mov	#17408,	r1	;#0x4400
    447c:	b2 40 80 5a 	mov	#23168,	&0x015c	;#0x5a80
    4480:	5c 01 
    4482:	d2 d3 04 02 	bis.b	#1,	&0x0204	;r3 As==01
    4486:	85 00 34 12 	mova	#0x01234,r5	
    448a:	03 43       	nop			
    448c:	03 43       	nop			
    448e:	86 00 78 56 	mova	#0x05678,r6	
    4492:	03 43       	nop			
    4494:	03 43       	nop			

    u 4486
    H sig=0x1381 addr=0x00004486 
    L sig=0x1381 addr=0x00004488 data=0x0085
    H sig=0x1201 addr=0x00004488               1
    L sig=0x1201 addr=0x0000448a data=0x1234
    H sig=0x5281 addr=0x0000448a               2
    L sig=0x5281 addr=0x0000448c data=0x4303
    H sig=0x9281 addr=0x0000448c               3
    L sig=0x9281 addr=0x0000448e data=0x4303
*/
static int write_register(uint32_t val, int n)
{
  if (n > 15)            panic();
  if (n == 0)            panic();
  //if (n == 2)   return write_register16(val);
  if (n == 3)            panic();
  
  n = (0x0080|((val>>8)&0x0F00)|n);
  set_pc(0x00f0);              // XXX is this OK??
  if (!cpu_instr_load())       led_message(".-- .-. . . .");

  IR_SHIFT(IR_DATA_16BIT);
  DR_SHIFT16(n);               /* mova #val, Rn */
  CLR_TCLK();
  SET_TCLK();                  /* 1 */

  if (cpu_instr_load())        led_message(".--. .-. .. .. ..");
  IR_SHIFT(IR_DATA_16BIT);
  DR_SHIFT16(val);
  CLR_TCLK();
  SET_TCLK();                  /* 2 */

  //if (!cpu_instr_load())       led_message(".--. .-. ... ... ...");
  IR_SHIFT(IR_DATA_16BIT);
  DR_SHIFT16(0x4303);

  CLR_TCLK();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x0501);
  SET_TCLK();                  /* 3 */

  while (!cpu_full_emulation()) {
    CLR_TCLK();
    SET_TCLK();
  }
  return OK;
}

/*
000044a2 <main>:
    44a2:       31 40 00 44     mov     #17408, r1      ;#0x4400
    44a6:       b2 40 80 5a     mov     #23168, &0x015c ;#0x5a80
    44aa:       5c 01 
    44ac:       60 01 fc 00     mova    r1,     &0x000fc
    44b0:       03 43           nop                     
    44b2:       03 43           nop                     
    44b4:       60 05 fc 00     mova    r5,     &0x000fc
    44b8:       03 43           nop                     
    44ba:       03 43           nop                     
    u 44ac
    H sig=0x9381 addr=0x000044ac 
    L sig=0x9381 addr=0x000044ae data=0x0160   
    H sig=0x9201 addr=0x000044ae                1
    L sig=0x9201 addr=0x000044b0 data=0x00fc   
    H sig=0x1281 addr=0x000044b0                2
    L sig=0x1280 addr=0x000000fc data=0x4303    
    H sig=0x9200 addr=0x000000fc                3
    L sig=0x9200 addr=0x000000fe data=0x4394   
    H sig=0x9200 addr=0x000000fe                4
    L sig=0x9201 addr=0x000044b2 data=0x0000   
    H sig=0x5281 addr=0x000044b2                5
*/ 

static uint32_t read_register(int n)
{
  uint16_t val_l;
  uint16_t val_h;

  if (n  > 15)         panic();
  if (n == 0)          panic();
  //if (n == 2) return read_register16();

  n = 0x0060|((n<<8)&0x0f00);  //  mova Rn, &xxxx, 4 cycles
  set_pc(0x00f0);              //  XXX is this OK??
  if (!cpu_instr_load()) led_message(".-. . --. . .");

  IR_SHIFT(IR_DATA_16BIT);
  DR_SHIFT16(n);  
  CLR_TCLK();                  
  SET_TCLK();                  /* 1 */

  if (cpu_instr_load())        led_message(".-. . --. . . .");
  IR_SHIFT(IR_DATA_16BIT);
  DR_SHIFT16(0x00fc);
  CLR_TCLK();
  SET_TCLK();                  /* 2 */

  if (!cpu_instr_load())       led_message(".-. . --. -. --- .--.");
  IR_SHIFT(IR_DATA_16BIT);
  DR_SHIFT16(0x4303);
  CLR_TCLK();
  SET_TCLK();                  /* 3 */

  if (cpu_instr_load())        led_message(".-. . --. .. .. ..");
  IR_SHIFT(IR_ADDR_CAPTURE);
  if (DR_SHIFT20(0) != 0x00fc) led_message(".-. . --. .- -.. -.. .-.");

  CLR_TCLK();
  IR_SHIFT(IR_DATA_CAPTURE);
  val_l = DR_SHIFT16(0);
  SET_TCLK();                  /* 4 */

  if (cpu_instr_load())        led_message(".-. . --. .... .... ....");
  if (!(cpuXv2_cntrl_sig()&SIG_CPUOFF)) { /* XXX, there's something wrong... */
    IR_SHIFT(IR_ADDR_CAPTURE);
    if (DR_SHIFT20(0) != 0x00fe) led_message(".-. . --. .- -.. -.. .-. . .");
  }
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x0501);

  CLR_TCLK();
  IR_SHIFT(IR_DATA_CAPTURE);
  val_h = DR_SHIFT16(0);
  SET_TCLK();                  /* 5 */

  while (!cpu_full_emulation()) {
    CLR_TCLK();
    SET_TCLK();  
  }
  return ((uint32_t)val_h<<16)+val_l;
}

/* ----------------------------------------------------------------------- */

static void save_regs()
{
  int    n;

  reg[3] = 0;                       /* constant generator */
  reg[2] = read_register(2);        /* SR */
  reg[1] = read_register(1);        /* SP */
  for (n = 15; n >= 4; n--)
    reg[n] = read_register(n);
}

static void restore_regs()
{
  int     n;

  for (n = 15; n >= 4; n--)
    write_register(reg[n], n);
  write_register(reg[2], 2);
  write_register(reg[1], 1);
  /* PC is set at continue() */
}

static uint16_t enter_full_emulation() 
{
  uint16_t    sig;

  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  sig = DR_SHIFT16(0x1501); /* (SIG_RELEASE_LBYTE0|SIG_TCE1|SIG_CPUSUSP|SIG_READ) */
  IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
  while ((DR_SHIFT16(0)&(SIG_TCE0|SIG_CPUSUSP))
         !=       (SIG_TCE0|SIG_CPUSUSP)) {
    CLR_TCLK();
    SET_TCLK();
  }
  if (sig&SIG_CPUOFF) {
    CLR_TCLK();
    SET_TCLK();
    CLR_TCLK();
    SET_TCLK();
    CLR_TCLK();
    SET_TCLK();
    CLR_TCLK();
    SET_TCLK();
    CLR_TCLK();
    SET_TCLK();
  } else {
    CLR_TCLK();
    SET_TCLK();
  }
  return sig;
}

uint32_t cpuXv2_stop_cpu()
{
  uint16_t    sig;

  sig = enter_full_emulation();

  IR_SHIFT(IR_ADDR_CAPTURE);
  reg[0] = DR_SHIFT20(0);
 
 /*
   *    XXX After 'LPMx;',  cpuXv2's address bus points 
   *    to next instruction.  Not sure for the case 
   *    CPUOFF is set during reti operation. 
   *    We mignt need nop after every 'LPMx;'.
   */
  if (sig&SIG_CPUOFF) reg[0] -= 2;

  save_regs();
  regs_saved = 1;
  /* wake CPU up for late memory operation  */
  write_register(0, 2); // set SR = 0

  return reg[0];
}

int cpuXv2_is_cpu_stopped()
{
  if (!regs_saved && (cpuXv2_cntrl_sig()&SIG_CPUOFF))
    cpuXv2_stop_cpu();
  return regs_saved;
} 

void cpuXv2_continue()
{
  restore_regs();
  cpuXv2_release_device(0);
}

/*
 *    When conditional jump fetched into pipline, cpuXv2 prefetches 
 *    instructions for both condition, we cannot break these prefetches.
 *
 *    L sig=0x1281 T- addr=0x0443e I data=0x9f0e   443c cmp r15, r14
 *    H sig=0x5281 T- addr=0x0443e I data=0x9f0e   
 *    L sig=0x5281 T- addr=0x04440 I data=0x3419   443e jge $+52 ; abs 0x4472
 *    H sig=0x5201 T- addr=0x04440 R data=0x533f   
 *    L sig=0x5201 T- addr=0x04472 R data=0x4d0f   4440 mov r13, r15
 *    H sig=0x9281 T- addr=0x04472 I data=0x4f0d  
 *    L sig=0x9281 T- addr=0x04442 I data=0x4d0f   4472 mov r13, r15
 *    H sig=0x1281 T- addr=0x04442 I data=0x533f   <- condition is not met.
 */
void cpuXv2_step_cpu(uint16_t n)
{
  void execute_instructions(int n) {
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT16(0x1401); /* (SIG_RELEASE_LBYTE0|SIG_TCE1|SIG_READ) */
    IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
    while (n > 0) {
      CLR_TCLK();
      SET_TCLK();
      if (DR_SHIFT16(0)&SIG_INSTR_LOAD) n--;
    }
  }

  int is_cond_jmp(uint16_t x) {
    return ((x&0xe000) == 0x2000) && ((x&0x1c00) != 0x1c00);
  }

  if (reg[2]&0x0010)  return;

  restore_regs();
  set_pc(reg[0]);

  execute_instructions(n);

 loop:
  IR_SHIFT(IR_ADDR_CAPTURE);
  reg[0] = DR_SHIFT20(0);

  CLR_TCLK();
  IR_SHIFT(IR_DATA_CAPTURE);
  if (is_cond_jmp(DR_SHIFT16(0))) {
    SET_TCLK();
    n = 2;
    IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
    if (DR_SHIFT16(0)&SIG_INSTR_LOAD) n--;
    do {
      CLR_TCLK();
      SET_TCLK();
      if (DR_SHIFT16(0)&SIG_INSTR_LOAD) n--;
    } while (n > 0);
    goto loop;
  }
  SET_TCLK();

  enter_full_emulation();
  save_regs();
  regs_saved = 1;
}

void cpuXv2_write_registers(uint32_t *r)
{
  int    n;

  if (!regs_saved) return;

  for (n = 0; n < 16; n++)
    reg[n] = r[n];
  reg[3] = 0; /* constant generator */
}

void cpuXv2_read_registers(uint32_t  *r)
{
  int    n;

  if (!regs_saved && cpu_full_emulation()) save_regs();

  for (n = 0; n < 16; n++) r[n] = reg[n];
  r[3] = 0;   /* constant generator */
}

void cpuXv2_write_register(int n, uint32_t val)
{
  if (!regs_saved)        return;
  if (n < 16 && n != 3)   reg[n] = val;
}

/* ----------------------------------------------------------------------- */

static int cpuXv2_execute_POR()
{
  int    i;

  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x0C01);         // POR TCE1 READ

  DR_SHIFT16(0x0401);         // TCE1 READ
  for (i=0; i < 5; i++) {
    CLR_TCLK();
    SET_TCLK();
  }
  DR_SHIFT16(0x0501);         // TCE1 CPUSUSP READ
  i = 0;
  do {
    if (i++ > 5) {
      led_message(".--. . -- ..-");
      return ERR;
    }
    CLR_TCLK();
    SET_TCLK();
  } while (!cpu_full_emulation());

  if (IR_SHIFT(IR_CNTRL_SIG_CAPTURE) != JTAG_ID) {
    led_message(".--. .. -..");
    return ERR;
  }
  cpuXv2_write_mem(16, 0x015c, 0x5a80); /* WDTCTL = (WDTPW|WDTHOLD), 
                                           XXX address hard coded   */
  save_regs();
  reg[0] = cpuXv2_read_mem(16, 0xfffe); /* set pc reset vector */
  regs_saved = 1;
  return OK;
}

int cpuXv2_get_device()
{
  int       i;

  regs_saved = 0;

  i = 0;
  do {
    if (i++ > 5) {
      led_message(".. -..");
      return ERR;
    }

    DETACH_TAP(); 
    ATTACH_TAP();
    if (cpuXv2_is_fuse_blown())  return ERR;

    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT16(0x0501);    /* TCE1|R/W  */

  } while (IR_SHIFT(IR_CNTRL_SIG_CAPTURE) != JTAG_ID);

  i = 0;
  while ((DR_SHIFT16(0)&(SIG_TCE0|SIG_CPUSUSP)) != (SIG_TCE0|SIG_CPUSUSP)) {
    if (i++ > 50) {
        led_message("... -.-- -. -.-.");
        return ERR;
    }
    CLR_TCLK();
    SET_TCLK();
  }
  IR_SHIFT(IR_COREIP_ID);
  coreip_id = DR_SHIFT16(0);

  /*
   *    IR_DEVICE_ID actually gives device descriptor base address
   */
  IR_SHIFT(IR_DEVICE_ID);  
  device_id_pointer = DR_SHIFT20(0);
  device_id_pointer = ((device_id_pointer&0x0ffff)<<4)
                     +((device_id_pointer&0x0f0000)>>16);

  /*
   *    device descriptor has to be accessed in ``quick'' mode,
   *    cpuXv2_read_mem(16, device_id_pointer+4) gives 0x3fff  
   */
  cpuXv2_read_mem_quick(device_id_pointer+4, &device_id, 1);
  device_id = (device_id>>8)+((device_id&0x00ff)<<8);

  if (cpuXv2_execute_POR() != OK) {
    led_message(".--. --- .-.");
    return ERR;
  }
  return OK;
}

int cpuXv2_release_device(uint32_t addr)
{
  switch (addr) {
  case 0x1b08: /* BOR */
    IR_SHIFT(IR_TEST_REG);
    DR_SHIFT16(0x0200);
    msleep(5);
    return OK;
  case 0xfffe: /* reset vector */
    IR_SHIFT(IR_CNTRL_SIG_16BIT);
    DR_SHIFT16(0x0C01);          // POR
    DR_SHIFT16(0x0401);
    IR_SHIFT(IR_CNTRL_SIG_RELEASE);
    return OK;
  case 0:      /* saved PC */
    set_pc(reg[0]);
    break;
  default:
    set_pc(addr);
  }
  regs_saved = 0;
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x0401);

  IR_SHIFT(IR_ADDR_CAPTURE);
  IR_SHIFT(IR_CNTRL_SIG_RELEASE);
  return OK;
}

/* ----------------------------------------------------------------------- */
#define inline_rdwr 
//#define inline_rdwr inline

static inline_rdwr void wr(uint16_t sig, uint32_t addr, uint16_t data)
{
  CLR_TCLK();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(sig);
  IR_SHIFT(IR_ADDR_16BIT);
  DR_SHIFT20(addr);

  SET_TCLK();
  IR_SHIFT(IR_DATA_TO_ADDR);
  DR_SHIFT16(data);
  CLR_TCLK();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x0501);
  SET_TCLK();
  CLR_TCLK();
  SET_TCLK();
}
static inline_rdwr void wrw(uint32_t addr, uint16_t data) {wr(0x0500, addr, data);}
static inline_rdwr void wrb(uint32_t addr, uint16_t data) {wr(0x0510, addr, data);}

static inline_rdwr uint16_t rd(uint16_t sig, uint32_t addr)
{
  uint16_t   x;

  CLR_TCLK();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(sig);
  IR_SHIFT(IR_ADDR_16BIT);
  DR_SHIFT20(addr);
  IR_SHIFT(IR_DATA_TO_ADDR);
  SET_TCLK();
  CLR_TCLK();
  x = DR_SHIFT16(0);
  SET_TCLK();
  CLR_TCLK();
  SET_TCLK();
  return x;
}
static inline_rdwr uint16_t rdw(uint32_t addr) {return rd(0x501, addr);}
static inline_rdwr uint16_t rdb(uint32_t addr) {return rd(0x511, addr);}

#undef inline_rdwr

/* ----------------------------------------------------------------------- */

void cpuXv2_write_mem(int nbits, uint32_t addr, uint16_t data) 
{
  if (!cpu_full_emulation()) {
    led_message(".-- .-. . -- ..-");
    return;
  }
  switch (nbits) {
  case 16:   wrw(addr, data); return;
  case  8:   wrb(addr, data); return;
  default: return;
  }
}

void cpuXv2_write_mem_quick(uint32_t addr, const uint16_t *buf, uint16_t len)
{
  if (!cpu_full_emulation()) {
    led_message(".-- .-. . -- ..-");
    return;
  }
  while (len-- > 0) {
    wrw(addr, *buf++);
    addr += 2;
  }
}

uint16_t cpuXv2_read_mem(int nbits, uint32_t addr)
{
  uint16_t  x;
  
  if (!cpu_full_emulation()) {
    led_message(". -- ..-");
    return ERR;
  }
  switch (nbits) {
  case 16: x = rdw(addr); break;
  case  8: x = rdb(addr); break;
  default: panic();
  }
  return x;
}

void  cpuXv2_read_mem_quick(uint32_t addr, uint16_t *buf, uint16_t len) 
{
  int    i;

  set_pc(addr);
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x0501);

  IR_SHIFT(IR_DATA_QUICK);
  for (i = 0; i < len; i++) {
    SET_TCLK();
    CLR_TCLK();
    *buf++ = DR_SHIFT16(0);
  }
  SET_TCLK();
  if (!cpu_full_emulation())
    led_message(".-. -.. -- . --");
}

void cpuXv2_write_mem_quickx(uint32_t addr, const uint16_t *buf, uint16_t len)
{
  set_pc(addr);
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x0500);

  IR_SHIFT(IR_DATA_QUICK);
  while (len-- > 0) {
    DR_SHIFT16(*buf++);
    SET_TCLK();
    CLR_TCLK();
  }
  SET_TCLK();
  if (!cpu_full_emulation())
    led_message(".-- .-. -- . --");
}

void cpuXv2_write_flash(uint32_t addr, const uint16_t *buf, uint16_t len)
{
  if (len == 0)              return;
  if (!cpu_full_emulation()) return;

  while (rdw(FCTL3)&0x01) ;          //  BUSY
  wrw(FCTL3, FCTL3_val);             
  wrw(FCTL1, 0xa560);                //  FWKEY|SWRT|WRT
  while (len-- > 0) {
    wrw(addr, *buf++);
    addr += 2;
  }
  wrw(FCTL1, FCTL3_val|0x0010);      //  |LOCK
}

void cpuXv2_erase_flash(int mode, uint32_t addr)
{
  if (!cpu_full_emulation()) return;
  while (rdw(FCTL3)&0x01) ;          //  BUSY
  wrw(FCTL1, mode);
  wrw(FCTL3, FCTL3_val);
  wrw(addr, 0);                      //  dummy write
  while (rdw(FCTL3)&0x01) ;          //  BUSY
  wrw(FCTL3, FCTL3_val|0x0010);      //  |LOCK
}

int cpuXv2_is_fuse_blown()
{
  int    i;
  
  for (i = 0; i < 3; i++) {   //  First trial could be wrong
    IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
    if (DR_SHIFT16(0xAAAA) == 0x5555) {
      return 1;               // Fuse is blown
    }
  }
  return 0;
}

/* ----------------------------------------------------------------------- */

#if 0

uint32_t cpuXv2_read_jmb()
{
  uint16_t    JMBOUT0, JMBOUT1;
  
  IR_SHIFT(IR_JMB_EXCHANGE);
  if (DR_SHIFT16(0)&OUT1RDY) {
    DR_SHIFT16(JMB32B|OUTREQ);
    JMBOUT0 = DR_SHIFT16(0);
    JMBOUT1 = DR_SHIFT16(0);
    return ((uint32_t)JMBOUT1<<16) + JMBOUT0;
  }
  return 0;
}

static int device_has_jtag17_error() {return 1;}

void cpuXv2_write_jmb(uint16_t m)
{
  uint16_t    JMBCTL;
  int         count;

  count = 0;

  if (device_has_jtag17_error()) {
    IR_SHIFT(IR_JMB_EXCHANGE);
    do { 
      JMBCTL = DR_SHIFT16(0);
      if (count++ > 3000) goto timeout;
    } while (!(JMBCTL&IN0RDY));
    JMBCTL = DR_SHIFT16(INREQ);
  } else {
    IR_SHIFT(IR_JMB_EXCHANGE);
    do { 
      JMBCTL = DR_SHIFT16(INREQ); // shift data into mailbox
      if (count++ > 3000) goto timeout;
    } while(!(JMBCTL&IN0RDY));
  }
  DR_SHIFT16(m);

 timeout:;
}

uint16_t cpuXv2_sync_jtag_assert_POR()
{
  int i;

  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(0x1501);            // Set device into JTAG mode + read
  if (IR_SHIFT(IR_CNTRL_SIG_CAPTURE) != JTAG_ID) return ERR;

  i = 0;
  while (!(DR_SHIFT16(0)&SIG_TCE0))
    if (i++ > 50) goto timeout;

  return cpuXv2_execute_POR();

 timeout:
  return ERR;
}

/* ----------------------------------------------------------------------- */

void cpuXv2_write_mem_quick_265e(uint32_t addr, const uint16_t *buf, uint16_t len)
{
  /* somewhat they don't use IR_DATA_QUICK here */
  while (len-- > 0) {
    cpuXv2_write_mem(16, addr, *buf++);
    addr += 2;
  }
}

/* http://srecord.sourceforge.net/ */
const uint16_t FLASH_WRITE[] = {
  0x001C, 0x00EE, 0xBEEF, 0xDEAD, 0xBEEF, 0xDEAD, 0xA508, 0xA508,
  0xA500, 0xA500, 0xDEAD, 0x000B, 0xDEAD, 0x000B, 0x40B2, 0x5A80,
  0x015C, 0x40B2, 0xABAD, 0x018E, 0x40B2, 0xBABE, 0x018C, 0x4290,
  0x0140, 0xFFDE, 0x4290, 0x0144, 0xFFDA, 0x180F, 0x4AC0, 0xFFD6,
  0x180F, 0x4BC0, 0xFFD4, 0xB392, 0x0144, 0x23FD, 0x4092, 0xFFBE,
  0x0144, 0x4290, 0x0144, 0xFFB8, 0x90D0, 0xFFB2, 0xFFB2, 0x2406,
  0x401A, 0xFFAA, 0xD03A, 0x0040, 0x4A82, 0x0144, 0x1F80, 0x405A,
  0xFF94, 0x1F80, 0x405B, 0xFF92, 0x40B2, 0xA540, 0x0140, 0x40B2,
  0x0050, 0x0186, 0xB392, 0x0186, 0x27FD, 0x429A, 0x0188, 0x0000,
  0xC392, 0x0186, 0xB392, 0x0144, 0x23FD, 0x1800, 0x536A, 0x1800,
  0x835B, 0x23F0, 0x1F80, 0x405A, 0xFF6C, 0x1F80, 0x405B, 0xFF6A,
  0xE0B0, 0x3300, 0xFF5C, 0xE0B0, 0x3300, 0xFF58, 0x4092, 0xFF52,
  0x0140, 0x4092, 0xFF4E, 0x0144, 0x4290, 0x0144, 0xFF42, 0x90D0, 
  0xFF42, 0xFF3C, 0x2406, 0xD0B0, 0x0040, 0xFF38, 0x4092, 0xFF34,
  0x0144, 0x40B2, 0xCAFE, 0x018E, 0x40B2, 0xBABE, 0x018C, 0x3FFF,
};

void cpuXv2_write_flash_265e(uint32_t addr, const uint16_t *buf, uint16_t len)
{
  uint16_t   loadAddr  = 0x1C00;                    // RAM start address of MSP430F5438
  uint16_t   startAddr = loadAddr + FLASH_WRITE[0]; // start address of the program in traget RAM
  uint32_t   Jmb;
  int        i, count;

  cpuXv2_write_mem_quick(loadAddr, FLASH_WRITE, sizeof(FLASH_WRITE)/sizeof(uint16_t));
  cpuXv2_write_mem(16, loadAddr+ 4, addr);
  cpuXv2_write_mem(16, loadAddr+ 6, addr>>16);
  cpuXv2_write_mem(16, loadAddr+ 8, len);
  cpuXv2_write_mem(16, loadAddr+10, 0); 
  cpuXv2_write_mem(16, loadAddr+12, 0xA508);  /* FCTL3: 0xA508 to unlock INFO Segment A. 
                                                        0xA548 to keep it unlocked   */
  cpuXv2_release_device(startAddr);
    
  count = 0;
  do {
    Jmb = cpuXv2_read_jmb();
    if (count++ > 3000) goto timeout;
  } while (Jmb != 0xABADBABE);

  while (len-- > 0)  cpuXv2_write_jmb(*buf++);

  count = 0;
  do {
    Jmb = cpuXv2_read_jmb();
    if (count++ > 3000) goto timeout;
  } while (Jmb != 0xCAFEBABE);

  cpuXv2_sync_jtag_assert_POR();

  for (i = 0; i < sizeof(FLASH_WRITE)/sizeof(uint16_t); i++) {
    cpuXv2_write_mem(16, loadAddr, 0x3fff);
    loadAddr += 2;
  }
 timeout:;
}

void cpuXv2_write_flash_265e_wo_release(uint32_t addr, const uint16_t *buf, uint16_t len)
{
  uint16_t loadAddr  = 0x1C00;                     // RAM start address of MSP430F5438
  uint16_t startAddr = loadAddr + FLASH_WRITE[0];  // start address of the program in traget RAM
  int i;

  cpuXv2_write_mem_quick(loadAddr, FLASH_WRITE, sizeof(FLASH_WRITE)/sizeof(uint16_t));
  cpuXv2_write_mem(16, loadAddr+ 4, addr);
  cpuXv2_write_mem(16, loadAddr+ 6, addr>>16);
  cpuXv2_write_mem(16, loadAddr+ 8, len);
  cpuXv2_write_mem(16, loadAddr+10, 0);
  cpuXv2_write_mem(16, loadAddr+12, 0xA548);  /* FCTL3: unlock INFO Segment A. 
                                                 0xA548 to keep it unlocked   */
  //deliver CPU clocks from JTAG
  set_pc(startAddr);
  /* 110  /105 clocks till signal 'wait for data' */
  for (i=0; i < 110; i++) {
    CLR_TCLK();
    SET_TCLK();
  }

  IR_SHIFT(IR_JMB_EXCHANGE);
  while (len-- > 0) {
    DR_SHIFT16(INREQ);
    DR_SHIFT16(*buf++);
    for (i=0; i < 30 ; i++) {
      CLR_TCLK();
      SET_TCLK();
    }
  }

  for (i=0; i < 110; i++) {
      CLR_TCLK();
      SET_TCLK();
  }

  cpuXv2_sync_jtag_assert_POR();

  for (i = 0; i < sizeof(FLASH_WRITE)/sizeof(uint16_t); i++) {
    cpuXv2_write_mem(16, loadAddr, 0x3fff); /* JMP $ */
    loadAddr += 2;
  }
}

static const uint16_t FLASH_ERASE[] = {
  0x0016, 0x00B0, 0xBEEF, 0xDEAD, 0xA502, 0xA508, 0xA508, 0xA500,
  0xA500, 0xDEAD, 0x000B, 0x40B2, 0x5A80, 0x015C, 0x4290, 0x0140,
  0xFFEE, 0x4290, 0x0144, 0xFFEA, 0x180F, 0x4AC0, 0xFFE6, 0xB392,
  0x0144, 0x23FD, 0x4092, 0xFFD4, 0x0144, 0x4290, 0x0144, 0xFFCE,
  0x90D0, 0xFFC8, 0xFFC8, 0x2406, 0x401A, 0xFFC0, 0xD03A, 0x0040,
  0x4A82, 0x0144, 0x1F80, 0x405A, 0xFFAC, 0x4092, 0xFFAC, 0x0140,
  0x40BA, 0xDEAD, 0x0000, 0xB392, 0x0144, 0x23FD, 0x1F80, 0x405A,
  0xFFA2, 0xE0B0, 0x3300, 0xFF98, 0xE0B0, 0x3300, 0xFF94, 0x4092,
  0xFF8E, 0x0140, 0x4092, 0xFF8A, 0x0144, 0x4290, 0x0144, 0xFF7E,
  0x90D0, 0xFF7E, 0xFF78, 0x2406, 0xD0B0, 0x0040, 0xFF74, 0x4092, 
  0xFF70, 0x0144, 0x40B2, 0xCAFE, 0x018E, 0x40B2, 0xBABE, 0x018C,
  0x3FFF,
};

void cpuXv2_erase_flash_265e(int mode, uint32_t addr)
{
  uint16_t   loadAddr  = 0x1C00;                    // RAM start address of MSP430F5438
  uint16_t   startAddr = loadAddr + FLASH_ERASE[0]; // start address of the program in target RAM
  uint32_t   Jmb;
  uint32_t   count;
  int i;

  cpuXv2_write_mem_quick(loadAddr, FLASH_ERASE, sizeof(FLASH_ERASE)/sizeof(uint16_t));
  cpuXv2_write_mem(16, loadAddr+ 4, addr);
  cpuXv2_write_mem(16, loadAddr+ 6, addr>>16);
  cpuXv2_write_mem(16, loadAddr+ 8, mode);
  cpuXv2_write_mem(16, loadAddr+10, 0xA548);  /* FCTL3: 0xA508 to unlock INFO Segment A. 
                                                        0xA548 to keep it locked   */
  cpuXv2_release_device(startAddr);

  count = 0;
  do {
    Jmb = cpuXv2_read_jmb();
    if (count++ > 3000) goto timeout;
  } while (Jmb != 0xCAFEBABE);

  cpuXv2_sync_jtag_assert_POR();

  for (i=0; i < sizeof(FLASH_ERASE)/sizeof(uint16_t); i++) {
    cpuXv2_write_mem(16, loadAddr, 0x3fff); /* JMP $ */
    loadAddr += 2;
  }
 timeout: ;
}

void cpuXv2_erase_flash_265e_wo_release(int mode, uint32_t addr)
{
  uint16_t   loadAddr  = 0x1C00;                    // RAM start address of MSP430F5438
  uint16_t   startAddr = loadAddr + FLASH_ERASE[0]; // start address of the program in target RAM
  int i;

  cpuXv2_write_mem_quick(loadAddr, FLASH_ERASE, sizeof(FLASH_ERASE)/sizeof(uint16_t));
  cpuXv2_write_mem(16, loadAddr+ 4, addr);
  cpuXv2_write_mem(16, loadAddr+ 6, addr>>16);
  cpuXv2_write_mem(16, loadAddr+ 8, mode);
  cpuXv2_write_mem(16, loadAddr+10, 0xA548);  /* FCTL3:  0xA508 to unlock INFO Segment A. 
                                                         0xA548 to keep it locked   */
  cpuXv2_release_device(startAddr);

  set_pc(startAddr);

  for (i=0; i < 110; i++) {
      CLR_TCLK();
      SET_TCLK();
  }

  //max mass/segment erase time for F543x is 32uS
  //do not check mailbox, just wait..
  usleep(35);

  for (i=0; i < 110; i++) {
      CLR_TCLK();
      SET_TCLK();
  }

  cpuXv2_sync_jtag_assert_POR();

  for (i = 0; i < sizeof(FLASH_ERASE)/sizeof(uint16_t); i++) {
    cpuXv2_write_mem(16, loadAddr, 0x3fff); /* JMP $ */
    loadAddr += 2;
  }
}

#endif

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
