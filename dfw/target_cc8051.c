/*
 *    Chipcon/TI CC111x/CC243x/CC251x series  
 *
 *                               written by skimu@mac.com
 *
 *    References:
 *    [1] CC1110/CC2430/CC2510 Debug and Programming Interface Specification,
 *        Rev. 1.2, SWRA124, Chipcon Products from Texas Instruments, 2006.
 *    [2] CC2510Fx/CC2511Fx Low-Power Soc (System-on-Chip) with MCU, Memory,
 *        2.4GHz RF Transceiver, and USB Controller, SWRS055F, 
 *        Texas Instruments, 2008
 */

#include "uif.h"
#include "uif_macros.h"
#include "target.h"
#include "target_cc8051.h"
#include "misc.h"

/* 
 * Pin assignments
 *   
 *    device pin   |   JTAG pin
 *   --------------+----------------------------------
 *    Debug Clock  |    TCK 
 *     Debug Data  |    TDI
 *        RESET_N  |    TGTRST (FET),  BTXD (eZ430U)
 *   --------------+----------------------------------
 */

/* Debug commands (Table 45 of SWRS055F) */

#define CHIP_ERASE          0x14          /* 0001 0100 */
#define WR_CONFIG           0x1d          /* 0001 1101 */
#define RD_CONFIG           0x24          /* 0010 0100 */
#define GET_PC              0x28          /* 0010 1000 */
#define READ_STATUS         0x34          /* 0011 0100 */
#define SET_HW_BRKPNT       0x3b          /* 0011 1011 */
#define HALT                0x44          /* 0100 0100 */
#define RESUME              0x4c          /* 0100 1100 */
#define DEBUG_INSTR0        0x54          /* 0101 0100 */
#define DEBUG_INSTR1        0x55          /* 0101 0101 */
#define DEBUG_INSTR2        0x56          /* 0101 0110 */
#define DEBUG_INSTR3        0x57          /* 0101 0111 */
#define STEP_INSTR          0x5c          /* 0101 1100 */
#define STEP_REPLACE0       0x64          /* 0110 0100 */
#define STEP_REPLACE1       0x65          /* 0110 0101 */
#define STEP_REPLACE2       0x66          /* 0110 0110 */
#define STEP_REPLACE3       0x67          /* 0110 0111 */
#define GET_CHIP_ID         0x68          /* 0110 1000 */

/* CHIP_ID (Table 5 of SWRA124) */

#define ID_CC1110           0x01
#define ID_CC2430           0x85
#define ID_CC2431           0x89
#define ID_CC2510           0x81
#define ID_CC2511           0x91

/* Flash Lock Protection Bits (Table 44 of SWRS055F) */

#define BBLOCK              0x10
#define LSIZE(x)            (((x)&7)<<1)
#define DGBLOCK             0x01

/* ------------------------------------------------------------------- */

static void shift_in(uint8_t d)
{
  int i;
  for (i=0; i < 8; i++) {
    set_TCK();
    if (d&0x80)
      set_TDI();
    else
      clr_TDI();
    clr_TCK();
    d<<=1;
  }
}

static uint8_t shift_out()
{
  int     i;
  uint8_t d;
  d = 0;
  clr_ENI2O();
  for (i=0; i < 8; i++) {
    set_TCK();
    if (tst_TDO())
      d = (d<<1) + 1;
    else
      d = (d<<1);
    clr_TCK();
  }
  set_ENI2O();
  return d;
}

/* ------------------------------------------------------------------- */

static int FLASH_WORD_SIZE = 2;

static uint16_t cc_get_chip_id()
{
  uint8_t     id, rev;

  shift_in(GET_CHIP_ID);
  id  = shift_out();
  rev = shift_out();
  if ((id == ID_CC2430)||(id == ID_CC2431))
    FLASH_WORD_SIZE =4;
  return (id<<8)+rev;
}

static void cc_resume()
{
  shift_in(RESUME);
  shift_out();
}

/* ------------------------------------------------------------------- */

void cc8051_wr_config(uint8_t c)
{
  shift_in(WR_CONFIG);
  shift_in(c);
  shift_out();
}

uint8_t cc8051_rd_config()
{
  shift_in(RD_CONFIG);
  return shift_out();
}

uint8_t cc8051_read_status()
{
  shift_in(READ_STATUS);
  return shift_out();
}

uint16_t cc8051_get_pc()
{
  uint8_t     pch, pcl;

  shift_in(GET_PC);
  pch = shift_out();
  pcl = shift_out();
  return (pch<<8)+pcl;
}

void cc8051_debug_instr1(uint8_t in1, uint8_t *out)
{
  shift_in(DEBUG_INSTR1);
  shift_in(in1);
  *out = shift_out();
}

void cc8051_debug_instr2(uint8_t in1, uint8_t in2, uint8_t *out)
{
  shift_in(DEBUG_INSTR2);
  shift_in(in1);
  shift_in(in2);
  *out = shift_out();
}

void cc8051_debug_instr3(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t *out)
{
  shift_in(DEBUG_INSTR3);
  shift_in(in1);
  shift_in(in2);
  shift_in(in3);
  *out = shift_out();
}

void cc8051_step_replace1(uint8_t in1, uint8_t *out)
{
  shift_in(STEP_REPLACE1);
  shift_in(in1);
  *out = shift_out();
}

void cc8051_step_replace2(uint8_t in1, uint8_t in2, uint8_t *out)
{
  shift_in(STEP_REPLACE2);
  shift_in(in1);
  shift_in(in2);
  *out = shift_out();
}

void cc8051_step_replace3(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t *out)
{
  shift_in(STEP_REPLACE3);
  shift_in(in1);
  shift_in(in2);
  shift_in(in3);
  *out = shift_out();
}

#define LOBYTE(w) ((uint8_t)(w))
#define HIBYTE(w) ((uint8_t)((w)>>8))

void cc8051_read_code(uint16_t addr, uint8_t bank, uint8_t *buf, uint16_t len)
{
  uint8_t dummy;

  cc8051_debug_instr3(0x75, 0xc7, bank*16+1, &dummy);           // MOV MEMCTR, (bank*16+1)
  cc8051_debug_instr3(0x90, HIBYTE(addr), LOBYTE(addr), &dummy);// MOV DPTR, addr
  while (len-->0) {
    cc8051_debug_instr1(0xe4, &dummy);                          // CLR  A
    cc8051_debug_instr1(0x93, buf++);                           // MOVC A,@A+DPTR
    cc8051_debug_instr1(0xa3, &dummy);                          // INC DPTR
  }
}

void cc8051_read_xdata(uint16_t addr, uint8_t *buf, uint16_t len)
{
  uint8_t dummy;
  cc8051_debug_instr3(0x90, HIBYTE(addr), LOBYTE(addr), &dummy);// MOV DPTR, addr
  while (len-->0) {
    cc8051_debug_instr1(0xe0, buf++);                           // MOVX A, @DPTR
    cc8051_debug_instr1(0xa3, &dummy);                          // INC DPTR
  }
}

void cc8051_write_xdata(uint16_t addr, const uint8_t *buf, uint16_t len)
{
  uint8_t dummy;
  cc8051_debug_instr3(0x90, HIBYTE(addr), LOBYTE(addr), &dummy);// MOV DPTR, addr
  while (len-->0) {
    cc8051_debug_instr2(0x74, *buf++, &dummy);                  // MOV  A, #imm
    cc8051_debug_instr1(0xf0, &dummy);                          // MOVX @DPTR, A
    cc8051_debug_instr1(0xa3, &dummy);                          // INC DPTR
  }
}

void cc8051_set_pc(uint16_t addr)
{
  uint8_t dummy;
  cc8051_debug_instr3(0x02, HIBYTE(addr), LOBYTE(addr), &dummy);
}

static const uint8_t routine[] = {
  /* 0*/  0x75, 0xad, 0,       //   MOV FADDRH, #imm ;(HIBYTE(addr/FLASH_WORD_SIZE))
  /* 3*/  0x75, 0xac, 0,       //   MOV FADDRL, #imm ;(LOBYTE(addr/FLASH_WORD_SIZE))

  /* 6*/  0x90, 0xf0, 0x00,    //   MOV DPTR, #0F000H

  /* 9*/  0x7f, 0,             //   MOV R7, #imm   ;(HIBYTE(len/FLASH_WORD_SIZE)+1)
  /*11*/  0x7e, 0,             //   MOV R6, #imm   ;(LOBYTE(len/FLASH_WORD_SIZE))
  /*13*/  0x75, 0xae, 0x02,    //   MOV FLC, #02H  ; WRITE

  /*16*/  0x7d, 0,             // L1:  MOV R5, #imm (FLASH_WORD_SIZE; 1,2,4)
  /*18*/  0xe0,                // L2:    MOVX A, @DPTR
  /*19*/  0xa3,                //        INC DPTR
  /*20*/  0xf5, 0xaf,          //        MOV FWDATA, A
  /*22*/  0xdd, 0xfa,          //        DJNZ R5, L2

  /*24*/  0xe5, 0xae,          // L3:    MOV A, FLC
  /*26*/  0x20, 0xe6, 0xfb,    //        JB  ACC_SWBSY, L3
  /*29*/  0xde, 0xf1,          //      DJNZ R6, L1
  /*31*/  0xdf, 0xef,          //      DJNZ R7, L1

  /*33*/  0xa5,                //   DB 0xA5 ; fake breakpoint
};

/* FLASH_WORD_SIZE:
 *    2    CC1110, CC1111, CC2510, CC2511
 *    4    CC2430, CC2431
 */

void cc8051_set_FLASH_WORD_SIZE(int x)
{
  FLASH_WORD_SIZE = x;
}

void cc8051_write_flash_page(uint32_t laddr, const uint8_t *buf, uint16_t len)
{
  uint8_t    dummy, bank;
  uint16_t   addr;

  bank = (laddr>>15)&0x03;
  addr = laddr&0x7fff;

  if (addr%FLASH_WORD_SIZE)      return;                   
  if (len < FLASH_WORD_SIZE)     return;
  if (len%FLASH_WORD_SIZE)       return;
  if (len/FLASH_WORD_SIZE > 512) return;

  cc8051_write_xdata(0xf000, buf, len);
  cc8051_write_xdata(0xf000+len, routine, sizeof(routine));
#define wrt(a,v) {dummy = v; cc8051_write_xdata(0xf000+len+a, &dummy, 1);}
  wrt( 2, HIBYTE(addr/FLASH_WORD_SIZE));
  wrt( 5, LOBYTE(addr/FLASH_WORD_SIZE));
  wrt(10, HIBYTE(len/FLASH_WORD_SIZE) + 1);
  wrt(12, LOBYTE(len/FLASH_WORD_SIZE));
  wrt(17, FLASH_WORD_SIZE);
#undef wrt
  cc8051_debug_instr3(0x75, 0xc7, bank*16+1, &dummy); // MOV MEMCTR, bank*16+1;
  cc8051_set_pc(0xf000+len);
  cc_resume();
  do {
    msleep(1);
  } while (!(cc8051_read_status()&CPU_HALTED));
}

void cc8051_mass_erase_flash()
{
  uint8_t dummy;
  cc8051_debug_instr1(0x00, &dummy);
  shift_in(CHIP_ERASE);
  shift_out();
  do {
    msleep(1);
  } while (!(cc8051_read_status()&CHIP_ERASE_DONE));
}

/* ------------------------------------------------------------------- */
/*           Public interface
 */
static uint16_t device_id = 0;

int      cc8051_device_id()             {return device_id;}
void     cc8051_set_FCTL3(uint16_t val) { /* dummy */}
uint16_t cc8051_cntrl_sig()             {return cc8051_read_status();}
int      cc8051_is_cpu_stopped()        {return cc8051_read_status()&CPU_HALTED;}

uint32_t cc8051_stop_cpu()
{
  shift_in(HALT);
  shift_out();
  while (!(cc8051_read_status()&CPU_HALTED)) ;
  return cc8051_get_pc();
}

void     cc8051_continue()      {cc_resume();}

void     cc8051_step_cpu(uint16_t n) {}; /* not implemented yet */

int      cc8051_get_device()
{
  uif_attach_cc8051();
  device_id = cc_get_chip_id();
  cc8051_wr_config(TIMERS_OFF|DMA_PAUSE|TIMER_SUSPEND);
  return OK;
}

int      cc8051_release_device(uint32_t addr)
{
  cc_resume();
  return OK;
}

void     cc8051_write_registers(uint32_t *regs){}      /* XXX */
void     cc8051_read_registers(uint32_t  *regs){}      /* XXX */
void     cc8051_write_register(int n, uint32_t val){}  /* XXX */
uint32_t cc8051_read_register(int n){return 0;}        /* XXX */

void     cc8051_write_mem(int nbits, uint32_t addr, uint16_t data)
{
  uint8_t   x;
  switch (nbits) {
  case  8: 
    x = data; 
    cc8051_write_xdata(addr, &x, 1);
    break;
  case 16:
    cc8051_write_xdata(addr, (uint8_t *)&data, 2);
    break;
  }
}

void     cc8051_write_mem_quick(uint32_t addr, const uint16_t *buf, uint16_t len)
{
  cc8051_write_xdata(addr, (uint8_t *)buf, len*2);
}

void     cc8051_write_flash(uint32_t addr, const uint16_t *buf, uint16_t len)
{
  uint16_t    page_size;
  uint8_t    *bp;
  int        i;
  uint8_t    xbuf[4];

  bp   = (uint8_t *)buf;   
  len *= 2;

  if (addr%FLASH_WORD_SIZE) {
    xbuf[0] = xbuf[1] = xbuf[2] = xbuf[3] = 0xff;
    while (addr%FLASH_WORD_SIZE && len > 0) {
      xbuf[addr%FLASH_WORD_SIZE] = *bp;
      addr++, bp++, len--;
    }
    cc8051_write_flash_page(addr-FLASH_WORD_SIZE, xbuf, FLASH_WORD_SIZE);
  }

  page_size = 512*FLASH_WORD_SIZE;
  while (len > page_size) {
    cc8051_write_flash_page(addr, bp, page_size);
    addr += page_size;
    bp   += page_size;
    len  -= page_size;
  }

  if (len%FLASH_WORD_SIZE) {
    cc8051_write_flash_page(addr, bp, len-len%FLASH_WORD_SIZE);
    addr += len - len%FLASH_WORD_SIZE;
    bp   += len - len%FLASH_WORD_SIZE;
    len = len%FLASH_WORD_SIZE;
    xbuf[0] = xbuf[1] = xbuf[2] = xbuf[3] = 0xff;
    for (i=0; len-- > 0; i++)  xbuf[i] = *bp++;
    cc8051_write_flash_page(addr, xbuf, FLASH_WORD_SIZE);
  } else {
    cc8051_write_flash_page(addr, bp, len);
  }

}

uint16_t cc8051_read_mem(int nbits, uint32_t addr)
{
  uint8_t  x;
  uint16_t y;

  switch (nbits) {
  case  8: cc8051_read_xdata(addr, &x, 1);  return x;
  case 16: cc8051_read_xdata(addr, (uint8_t *)&y, 2); return y;
  }
  return 0;
}

void     cc8051_read_mem_quick(uint32_t addr, uint16_t *buf, uint16_t len)
{
  cc8051_read_xdata(addr, (uint8_t *)buf, len*2);
}

void     cc8051_erase_flash(int mode, uint32_t addr)
{
  switch (mode) {
  case cc8051_ERASE_GLOB:
  case cc8051_ERASE_ALLMAIN:
  case cc8051_ERASE_MASS:
  case cc8051_ERASE_MAIN:
    cc8051_mass_erase_flash(); break;
  case cc8051_ERASE_SGMT:
    /* XXX */                  break;
  }
}

int      cc8051_is_fuse_blown()
{
  return cc8051_read_status()&DEBUG_LOCKED;
}

/* EOF */
