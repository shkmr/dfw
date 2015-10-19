/* -------------- command interpreter ---------------- */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uif.h"
#include "datalink.h"
#include "tap.h"
#include "target.h"
#include "target_msp430.h"
#include "target_cc8051.h"
#include "target_cpuXv2.h"
#include "misc.h"
//#include "eeprom.h"

int debug = 0;

/* ---------------------------------------------------------------------- */
/*   Use upper case only for bulk transfer hexnums.
 */
int CTOH(x) {return x >'9' ?  x - 'A' + 0x0A : x - '0';}
int HTOC(x) {return x > 9  ?  x - 0x0A + 'A' : x + '0';}

static int isxdigit(int c)
{
  return ((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F'));
}

static int gethex()
{
  int x, y;
  if (!isxdigit(x=getchar())) return -1;
  if (!isxdigit(y=getchar())) return -1;
  return (CTOH(x)<<4) + CTOH(y);
}

static void puthex(int h)
{
  int x, y;
  x = ((h>>4)&0x0f);
  y = (h&0x0f);
  putchar(HTOC(x));
  putchar(HTOC(y));
}

/* ---------------------------------------------------------------------- */

static void wait_for_nl()    {while (getchar() != '\n') ;}
static void error_occured()  {wait_for_nl(); putchar('N'); putchar('\n');}
static void ready_for_data() {wait_for_nl(); putchar('K'); putchar('\n');}

static void hexwrite_line(uint32_t addr, uint8_t *src, int len)
{
  uint8_t   checksum;
  int       i;
  void      puth(uint8_t x) { puthex(x); checksum -= x;}

  checksum = 0;
  putchar(':');
  if ((addr+len) > 0x10000) {
    puth(8);
    puth(len);
    puth(addr>>24);
    puth(addr>>16);
    puth(addr>>8);
    puth(addr);
  } else {
    puth(0);
    puth(len);
    puth(addr>>8);
    puth(addr);
  }
  for (i=0; i < len; i++) puth(src[i]);
  puth(checksum);
  putchar('\n');
  wait_for_nl();
}

#if 0 
/* XXX: -O produces wrong code with enum, -O2 is OK (gcc-3.2.3) */
enum {
  HEXWRITE_MAX     =   32,
  HEXREAD_MAX      =   64,
};
#else
static const int HEXWRITE_MAX  = 32;
static const int HEXREAD_MAX   = 64;
#endif

static void hexwrite(uint32_t addr, uint8_t *src, uint32_t len)
{
  while (len > HEXWRITE_MAX) {
    hexwrite_line(addr, src, HEXWRITE_MAX);
    addr += HEXWRITE_MAX;
    src  += HEXWRITE_MAX;
    len  -= HEXWRITE_MAX;
  }
  hexwrite_line(addr, src, len);
  putstr(":01000000FF\n");
}

static void hexread(void (*copyout)(uint32_t addr, uint8_t *buf, int len))
{
  uint8_t    type, len,checksum;
  uint8_t    buf[HEXREAD_MAX];
  uint32_t   addr;
  int        i;

  while (1) {
    int    geth() {int c = gethex(); checksum += c; return c;}

    ready_for_data();
    if (getchar() != ':') goto error;
    checksum = 0;

    type = geth();
    len  = geth();
    if (len > HEXREAD_MAX) goto error;

    addr = geth();
    addr = (addr<<8) + geth();
    if (type == 8) {
      addr = (addr<<8) + geth();
      addr = (addr<<8) + geth();
    }
    for (i=0; i < len; i++) buf[i] = geth();

    checksum += gethex();
    if (checksum&0xff) goto error;
    switch (type) {
    case 0:
    case 8:
      copyout(addr, buf, len);
      break;
    case 1: return;
    default:
      goto error;
    }
  }
 error:
  error_occured();
}

/* ------------------------------------------------------------- */

enum {
  TOKEN_MAX  = 16
};

/* XXX name is wrong.
 *   connect() interprets option string and get device if get_dev != 0.
 */
static void connect(int get_dev)
{
  char     str[TOKEN_MAX];
  int      vcct = 3300;
  int      its(char *x) { return strncmp(str, x, TOKEN_MAX) == 0;}
  void     getit()      { gettoken(str, TOKEN_MAX);}

  for (getit(); str[0] != '\0'; getit()) {
         if (its("DEBUG") )      debug++;
    else if (its("MSP430"))      target_set_msp430();
    else if (its("CC8051"))      target_set_cc8051();
    else if (its("CPUXV1"))      target_set_cpuX();
    else if (its("CPUXV2"))      target_set_cpuXv2();
    else if (its("JTAG")  )      tap_set_jtag();
    else if (its("SBW")   )      tap_set_sbw();
    else if (its("FastFlash"))   msp430_set_FastFlash();
    else if (its("SlowFlash"))   msp430_set_SlowFlash();
    else if (its("UNLOCKA"))     target_set_FCTL3(0xa540);
    else if (its("LOCKA"))       target_set_FCTL3(0xa500);
    else if (its("VCC")   ) {

      gettoken(str, TOKEN_MAX);
      vcct = strtol(str, NULL, 10);

    } else if (its("FCTL3"))  {

      gettoken(str, TOKEN_MAX);
      target_set_FCTL3(strtol(str, NULL, 16));
      
    } else if (its("FCTL"))   {

      gettoken(str, TOKEN_MAX);
      cpuXv2_set_FCTL_base(strtol(str, NULL, 16));

    } else if (its("FWS")   ) {

      gettoken(str, TOKEN_MAX);
      cc8051_set_FLASH_WORD_SIZE(strtol(str, NULL, 10));

    }
  }
  wait_for_nl();
  vcct = uif_set_vcct(vcct);
  if (get_dev) {
    uif_reset_target();
    target_get_device();
  }
  printf("K VCCT=%d, DeviceId=0x%04x\n", 
         vcct, target_device_id());
}

static void disconnect()
{
  wait_for_nl();
  target_release_device(0xfffe);
  putstr("K\n");
}

static void fet_state()
{
  wait_for_nl();
  printf("K\n");
}

static void erase_flash()
{
  char         str[TOKEN_MAX];
  uint16_t     mode;
  uint16_t     addr;

  int  its(char *x) {return strncmp(str, x, TOKEN_MAX) == 0;}

  gettoken(str, TOKEN_MAX);

  mode = ERASE_MASS;
       if (its("GLOB")) mode = ERASE_GLOB;
  else if (its("ALLM")) mode = ERASE_ALLMAIN;
  else if (its("MASS")) mode = ERASE_MASS;
  else if (its("MAIN")) mode = ERASE_MAIN;
  else if (its("SGMT")) mode = ERASE_SGMT;

  gettoken(str, TOKEN_MAX);
  addr = strtol(str, NULL, 16);
  wait_for_nl();
  target_erase_flash(mode, addr);
  putstr("K\n");
}

static void cpu_state() 
{
  uint16_t     sig;

  wait_for_nl();
  sig = target_cntrl_sig();
  if (target_is_cpu_stopped())
    printf("STOPPED 0x%04x\n", sig);
  else
    printf("RUNNING 0x%04x\n", sig);
}

static void stop_cpu()
{
  uint32_t      addr;
  uint16_t      sig;

  wait_for_nl();
  addr = target_stop_cpu();
  sig  = target_cntrl_sig();
  printf("K 0x%08lx 0x%04x\n", addr, sig);
}

static void continue_cpu()
{
  wait_for_nl();
  target_continue();
  printf("K 0x%04x\n", target_cntrl_sig());
}

static void step_cpu()
{
  uint16_t    n;
  char        str[TOKEN_MAX];

  gettoken(str, TOKEN_MAX);
  n = strtol(str, NULL, 10);
  wait_for_nl();
  target_step_cpu(n);
  printf("K 0x%04x\n", target_cntrl_sig());
}

static void read_registers()
{
  uint32_t    reg[16];

  wait_for_nl();
  target_read_registers(reg);
  hexwrite(0, (uint8_t *)reg, 64);
  printf("\n");
}

static void write_register()
{
  int         n;
  uint32_t    v;
  char        str[TOKEN_MAX];

  gettoken(str, TOKEN_MAX);
  n = strtol(str, NULL, 10);
  gettoken(str, TOKEN_MAX);
  v = strtol(str, NULL, 16);
  wait_for_nl();
  target_write_register(n, v);
  printf("K\n");
}

#if 0 
static void read_eeprom()
{
  uint32_t    addr, len;
  uint8_t     buf[HEXWRITE_MAX];
  char        str[TOKEN_MAX];

  gettoken(str, TOKEN_MAX);
  addr = strtol(str, NULL, 16);
  gettoken(str, TOKEN_MAX);
  len  = strtol(str, NULL, 16);
  wait_for_nl();

  while (len > HEXWRITE_MAX) {
    eeprom_read(addr, buf, HEXWRITE_MAX);
    hexwrite_line((uint32_t)addr, buf, HEXWRITE_MAX);
    addr += HEXWRITE_MAX;
    len  -= HEXWRITE_MAX;
  }
  eeprom_read(addr, buf, len);
  hexwrite_line((uint32_t)addr, buf, len);
  putstr(":01000000FF\n");
}

static void wreep(uint32_t addr, uint8_t *buf, int len)
{
  eeprom_write(addr, buf, len);
}

static void write_eeprom()
{
  hexread(wreep);
}
#endif

static void read_memory()
{
  uint32_t    addr, len;
  uint8_t     buf[HEXWRITE_MAX];
  char        str[TOKEN_MAX];

  gettoken(str, TOKEN_MAX);
  addr = strtol(str, NULL, 16);

  gettoken(str, TOKEN_MAX);
  len  = strtol(str, NULL, 16);

  wait_for_nl();

  if (len > 0 && addr%2 == 1) {
    buf[0] = target_read_mem(8,addr);
    hexwrite_line((uint32_t)addr, buf, 1);
    len--, addr++;
  }
  while (len > 32) {
    target_read_mem_quick(addr, (uint16_t *)buf, HEXWRITE_MAX/2);
    hexwrite_line((uint32_t)addr, buf, HEXWRITE_MAX);
    addr += HEXWRITE_MAX;
    len  -= HEXWRITE_MAX;
  }
  target_read_mem_quick(addr, (uint16_t *)buf, len/2+len%2);
  hexwrite_line((uint32_t)addr, buf, len);
  putstr(":01000000FF\n");
}

static void wrmem(uint32_t addr, uint8_t *buf, int len)
{
  if (addr%2 == 0) {
    target_write_mem_quick((uint16_t)addr, (uint16_t *)buf, len/2);
    if (len%2 == 1) {
      target_write_mem(8, addr+len, buf[len-1]);
    }
  } else {
    target_write_mem(8, addr, buf[0]);
    target_write_mem_quick(addr+1, (uint16_t *)(buf+1), (len-1)/2);
    if ((len-1)%2 == 1) {
      target_write_mem(8, addr+len-1, buf[len-1]);
    }
  }
}

static void write_memory()
{
  hexread(wrmem);
}

static void wrf(uint32_t addr, uint8_t *buf, int len)
{
  uint8_t  x[2];

  target_write_flash((uint16_t)addr, (uint16_t *)buf, len/2);
  if (len%2) {
    x[0] = *(buf+len-1);
    x[1] = 0xff;
    target_write_flash((uint16_t)addr+len-1, (uint16_t *)x, 1);
  }
}

static void write_flash()
{
  hexread(wrf);
}

void enter_updater(void); /* updater.c */

static void interactive_mode();

void run()
{
  while (1) {
    switch (getchar()) {
    case '\n': printf("*\n");       break;
    case 'b':  
      wait_for_nl(); 
      enter_updater(); 
      break;
    case 'c': connect(1);           break;
    case 'd': disconnect();         break;
    case 'e': erase_flash();        break;
    case 'f': write_flash();        break;
    case 'g': debug++;              break;
    case 'h': fet_state();          break;
    case 'i': interactive_mode();   break;

    case 'j': cpu_state();          break;
    case 'k': stop_cpu();           break;
    case 'l': continue_cpu();       break;
    case 'm': step_cpu();           break;

    case 'o': connect(0);           break;

    case 'p': read_registers();     break;
    case 'q': write_register();     break;
 
    case 'r': read_memory();        break;
    case 's': write_memory();       break;

    //case 'u': read_eeprom();      break;
    //case 'v': write_eeprom();     break;

    default:  wait_for_nl(); printf("*\n");
    }
  }
}

/* --------------------------------------------------------------------------- */

static void test_write_mem()
{
  uint32_t   addr;
  uint16_t   x;
  char       str[TOKEN_MAX];

  gettoken(str, TOKEN_MAX);
  addr = strtol(str, NULL, 16);

  gettoken(str, TOKEN_MAX);
  x = strtol(str, NULL, 16);

  wait_for_nl();
  cpuXv2_write_mem(16, addr, x);
  printf("K\n");
  return;
}

static void test_read_regs()
{
  uint32_t   reg[16];
  int        n;

  wait_for_nl();
  target_read_registers(reg);
  printf("%04lx %04lx %04lx", reg[0], reg[1], reg[2]);
  for (n = 4; n < 16; n++)  printf(" %04lx", reg[n]);
  printf("\n");
}

static void test_write_reg()
{
  int        n;
  uint16_t   v;
  char       str[TOKEN_MAX];

  gettoken(str, TOKEN_MAX);
  n = strtol(str, NULL, 10);

  gettoken(str, TOKEN_MAX);
  v = strtol(str, NULL, 16);

  wait_for_nl();
  target_write_register(n, v);
  printf("CNTRL_SIG 0x%04x\n", target_cntrl_sig());
}

#if   0                           /* DEBUG_CPUXV2 */

#include   "jtag91.h"

#define    debug_set_pc(x)        cpuXv2_set_pc(x)
#define    ADDR_SHIFT(x)          DR_SHIFT20(x)

void       cpuXv2_set_pc(uint32_t addr);

#elif 1                           /* DEBUG_CPUX */

#include   "jtag89.h"

#define    SIG_TCE0               SIG_TCE
#define    debug_set_pc(x)        cpuX_set_pc(x)
#define    ADDR_SHIFT(x)          DR_SHIFT20(x)

void       cpuX_set_pc(uint32_t addr);

#else                             /* DEBUG_MSP430 */

#include   "jtag89.h"

#define    SIG_TCE0               SIG_TCE
#define    debug_set_pc(x)        msp430_set_pc(x)
#define    ADDR_SHIFT(x)          DR_SHIFT16(x)

void       msp430_set_pc(uint32_t addr);

#endif                            /* ------------ */

static int tclk = 1;

static void print_bus()
{
  uint16_t    sig, data;
  uint32_t    addr;

  IR_SHIFT(IR_CNTRL_SIG_CAPTURE);
  sig = DR_SHIFT16(0);

  IR_SHIFT(IR_ADDR_CAPTURE);
  addr = ADDR_SHIFT(0);

  IR_SHIFT(IR_DATA_CAPTURE);
  data = DR_SHIFT16(0);

  printf("%c sig=0x%04x %c%c addr=0x%05lx %c data=0x%04x\n", 
         tclk == 0         ? 'L' : 'H',
         sig,
         (sig&SIG_TCE0)    ? 'T' : '-',
         (sig&SIG_CPUOFF)  ? 'O' : '-',
         addr, 
         (sig&SIG_INSTR_LOAD) ? 'I'
           : (sig&SIG_BYTE) ? ((sig&SIG_READ) ? 'r' : 'w')
             : ((sig&SIG_READ) ? 'R' : 'W'),
         data);
}

static void test_steph()
{
  wait_for_nl();
  if (tclk == 1) {
    CLR_TCLK(); tclk = 0; print_bus();
  } else {
    SET_TCLK(); tclk = 1; print_bus();
  }
}

static void test_step()
{
  wait_for_nl();
  if (tclk == 1) {
    CLR_TCLK(); SET_TCLK(); print_bus();
  } else {
    SET_TCLK(); tclk = 1;   print_bus();
  }
}

static void test_sig()
{
  uint16_t   sig;
  char       str[TOKEN_MAX];

  gettoken(str, TOKEN_MAX);
  sig = strtol(str, NULL, 16);

  wait_for_nl();
  IR_SHIFT(IR_CNTRL_SIG_16BIT);
  DR_SHIFT16(sig); 
  print_bus();
}

static void test_data()
{
  uint16_t   data;
  char       str[TOKEN_MAX];

  gettoken(str, TOKEN_MAX);
  data = strtol(str, NULL, 16);

  wait_for_nl();
  IR_SHIFT(IR_DATA_16BIT);
  DR_SHIFT16(data); 
  print_bus();
}

static void test_set_pc()
{
  uint16_t   data;
  char       str[TOKEN_MAX];

  gettoken(str, TOKEN_MAX);
  data = strtol(str, NULL, 16);

  debug_set_pc(data);
  wait_for_nl();
  tclk = 1;
  print_bus();
}

static void interactive_mode()
{
  wait_for_nl();
  printf("#\n");
  while (1) {
    switch (getchar()) {
    case '\n' : printf("#\n");     break;

    case 'h'  : test_steph();       break;

    case 'j'  : cpu_state();  break;

    case 'm'  : test_write_mem();  break;

    case 'p'  : continue_cpu();    break;
    case 'q'  : stop_cpu();        break;
    case 'r'  : test_read_regs();  break;

    case 's'  : test_sig();        break;
    case 't'  : test_step();       break;
    case 'u'  : test_set_pc();     break;
    case 'v'  : test_data();       break;

    case 'w'  : test_write_reg();  break;
    case 'z'  : return;
    default: ;
    }
  }
}

/* EOF */
