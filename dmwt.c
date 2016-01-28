/* 
 *    Flash programmer for DFW
 *                        written by skimu@mac.com 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <err.h>
#include <sys/time.h>

#include "segment.h"
#include "dfw/libdfw.h"

static int debug = 1;
static int firmware = 0;

msg(char *m) {fputs(m, stderr);}

/* -------------------------------------------------------------------------- */

#if 1
hexdump1(int addr, uint8_t *data, int len)
{
  printf("%04x", addr);
  while (len-- > 0)
    printf(" %02x", *data++);
  printf("\n");
}

hexdump(int addr, uint8_t *data, int len)
{
  const int ncol = 16;

  while (len > ncol) {
    hexdump1(addr, data, ncol);
    addr += ncol;
    data += ncol;
    len  -= ncol;
  }
  hexdump1(addr, data, len);
}
#else
/* taken from mspdebug-0.4 */
void hexdump(int addr, const u_int8_t *data, int len)
{
	int offset = 0;

	while (offset < len) {
		int i, j;

		/* Address label */
		printf("    %04x:", offset + addr);

		/* Hex portion */
		for (i = 0; i < 16 && offset + i < len; i++)
			printf(" %02x", data[offset + i]);
		for (j = i; j < 16; j++)
			printf("   ");

		/* Printable characters */
		printf(" |");
		for (j = 0; j < i; j++) {
			int c = data[offset + j];

			printf("%c", (c >= 32 && c <= 126) ? c : '.');
		}
		for (; j < 16; j++)
			printf(" ");
		printf("|\n");

		offset += i;
	}
}
#endif

/* ---------------------------------------------------------------------------- */

typedef struct timeval timeval_t;

int time_start(timeval_t *tv)
{
  return gettimeofday(tv, NULL);
}

int time_report(char *str, timeval_t *tv)
{
  double    s, t;

  s = tv->tv_sec + tv->tv_usec/1e6;
  gettimeofday(tv, NULL);
  t = tv->tv_sec + tv->tv_usec/1e6;
  printf("%s took %f seconds\n", str, t-s);
}

/* --------------------------------------------------------------------*/

#define VERIFY_MATCH 0
#define MAX_SECTION 65536
static uint8_t targetbuf[MAX_SECTION];

void read_mem(uint32_t addr, uint8_t *buf, int32_t len)
{
  if (firmware == 0)
    dfw_target_read_memory(addr, buf, len);
  else
    dfwup_read_memory(addr, buf, len);
}

int verify_segment(segment_t *seg)
{
  if (segment_size(seg) > MAX_SECTION)
    errx(1, "section size (%d) is too large", segment_size(seg));

  read_mem(segment_addr(seg), targetbuf, segment_size(seg));

  return 
    memcmp(segment_buf(seg), targetbuf, segment_size(seg)) == 0 
    ? VERIFY_MATCH : !VERIFY_MATCH;
}

int verify_all_segments()
{
  segment_t   *seg;

  for (seg = segment_start(); seg != NULL; seg = segment_next(seg)) {
    msg(".");
    if (verify_segment(seg) != VERIFY_MATCH)
      return !VERIFY_MATCH;
  }
  return VERIFY_MATCH;
}

void load_segment(segment_t *seg)
{
  if (segment_size(seg) > MAX_SECTION)
    errx(1, "section size (%d) is too large", segment_size(seg));
  if (segment_size(seg) == 0)
    warnx("section size is zero");

  dfw_target_write_flash(segment_addr(seg),
			 segment_buf(seg),
			 segment_size(seg));
}

update()
{
  timeval_t    tv;
  
  time_start(&tv);
  if (verify_all_segments() == VERIFY_MATCH) {
    msg("\nprogram matches with the target, no need to update\n");
    dfw_close_target();
    return;
  }
  time_report("\nIt", &tv);

  msg("OK, the program needs to be updated.\n");
  time_start(&tv);
  msg("Erasing flash..\n");
  dfw_target_erase_flash(ERASE_MASS, 0xfff0);
  time_report("It", &tv);

  time_start(&tv);
  {
    segment_t *seg;

    msg("programming");
    for (seg = segment_start(); seg != NULL; seg = segment_next(seg)) {
      msg(".");
      load_segment(seg); 
      msg(".");
      if (verify_segment(seg) != VERIFY_MATCH)
	errx(1, "verify error in section start from 0x%04x",
             segment_addr(seg));
    }
    msg(" done\n");
  }
  time_report("It", &tv);
}

target_update(char *options)
{
  dfw_open_target(options);
  msg("examining current version of the target device.");
  update();
  dfw_close_target();
}

firmware_update()
{
  dfw_enter_updater();
  msg("examining firmware version.");
  update();
  dfwup_PUC();
}
/* ------------------------ backup ------------------------------------ */

txt_write_line(FILE *fp, uint8_t *buf, int len)
{
  while (len-- > 0)
    fprintf(fp, "%02X ", *buf++);
  fprintf(fp, "\n");
}

txt_write(FILE *fp, uint32_t addr, uint8_t *buf, int len)
{
  const int NB = 16;

  fprintf(fp, "@%04X\n", (int)addr);
  while (len > NB) {
    txt_write_line(fp, buf, NB);
    len -= NB;
    buf += NB;
  }
  txt_write_line(fp, buf, len);
}

txt_write_finish(FILE *fp)
{
  fprintf(fp, "q\n");
}

backup(char *options, char *fname, uint32_t a1, uint32_t a2)
{
  FILE        *fp;
  timeval_t    tv;

  int len = a2 - a1 + 1;

  if (len < 0) errx(1, "from address (0x%08) is too large!", a1);

  fprintf(stderr, "backing up from 0x%04x to 0x%04x into %s\n", a1, a2, fname);

  time_start(&tv);
  dfw_open_target(options);
  dfw_target_read_memory(a1, targetbuf, len);
  time_report("It", &tv);

  if ((fp = fopen(fname, "w")) == NULL)
    errx(1, "can't open %s for write", fname);

  txt_write(fp, a1, targetbuf, len);
  txt_write_finish(fp);

  fclose(fp);
}

/* -------------------- TITXT reader ------------------------------------- */

uint8_t *txt_buf      = targetbuf;
int      txt_addr     = 0;
int      txt_count    = 0;

txt_set_addr(int x)
{
  segment_t *seg;
  if (txt_count > 0) {
    seg = segment_new(txt_addr, txt_count);
    memcpy(segment_buf(seg), txt_buf, txt_count);
  }
  txt_addr = x;
  txt_count = 0;
}

txt_put_byte(int x)
{
  txt_buf[txt_count++] = x;
  if (txt_count > MAX_SECTION)
    errx(1, "titxt section too large");
}

txt_read(char *fname)
{
  extern titxt_read();
  FILE *fp;
 
  if ((fp = fopen(fname, "r")) == NULL)
    errx(1, "can't open %s", fname);

  txt_count = 0;
  titxt_read(fp, txt_set_addr, txt_put_byte);
  txt_set_addr(0);

  fclose(fp);
}

/* ----------------------- IHEX reader --------------------------------- */

#include "hex.h"

uint8_t   *ihex_buf     = targetbuf;
int        ihex_addr;
int        ihex_count;
uint8_t   *ihex_ptr;

void ihex_out(void *param, unsigned int addr, unsigned int len, unsigned char *data)
{
  segment_t    *seg;

  if (ihex_addr == -1) ihex_addr = addr;

  if (addr != ihex_addr + ihex_count) {
    seg = segment_new(ihex_addr, ihex_count);
    memcpy(segment_buf(seg), ihex_buf, ihex_count);
    ihex_addr  = addr;
    ihex_count = 0;
    ihex_ptr   = ihex_buf;
  }
  memcpy(ihex_ptr, data, len);
  ihex_ptr   += len;
  ihex_count += len;
  if (ihex_count > MAX_SECTION)
    errx(1, "ihex section too large");

}

ihex_read(char *fname)
{
  FILE     *fp;

  if ((fp = fopen(fname, "r")) == NULL)
    errx(1, "can't open %s", fname);

  ihex_addr = -1;
  ihex_count = 0;
  ihex_ptr = ihex_buf;
  hex_read(fp, ihex_out, NULL);
  ihex_out(NULL, -1, 0, NULL);

  fclose(fp);
}

/* ---------------------- object reader -------------------------------- */

enum {
  FORMAT_UNKNOWN,
  FORMAT_ELF,
  FORMAT_TITXT,
  FORMAT_IHEX,
};

int obj_format(char *fname)
{
  FILE     *fp;
  int       f;

  if ((fp = fopen(fname, "rb")) == NULL)
    errx(1, "can't open %s", fname);
  switch (fgetc(fp)) {
  case 0x7f: f = FORMAT_ELF;    break;
  case '@' : f = FORMAT_TITXT;  break;
  case ':' : f = FORMAT_IHEX;   break;
  default:   f = FORMAT_UNKNOWN;
  }
  fclose(fp);
  return f;
}

int obj_read(char *fname)
{
  extern elf_read(char *name); /* elfbfd.c */

  switch (obj_format(fname)) {
  case FORMAT_ELF:   elf_read(fname);  break;
  case FORMAT_TITXT: txt_read(fname);  break;
  case FORMAT_IHEX:  ihex_read(fname); break;
  default :
    errx(1, "unknown object format %d", fname);
  }
}

/* ---------------------------------------------------------------------- */

void cleanup()
{
  libdfw_finish();
}

/* --------------------------------------------------------------------- */

debug_run()
{
  char     buf[4096];
  int      i;

  dfw_enter_updater();
  memset(buf, 0, 4096);
  if (dfwup_read_memory(0xc000, buf, 32) != DFW_OK) msg("err\n");
  hexdump(0xc000, buf, 32);
  memset(buf, 0, 4096);
  if (dfwup_read_memory(0xd000, buf, 32) != DFW_OK) msg("err\n");
  hexdump(0xd000, buf, 32);
  for (i=0; i < 16; i++) {
    buf[i] = i;
  }
  if (dfwup_write_flash(0xd000, buf, 32) != DFW_OK) msg("err\n");
  if (dfwup_read_memory(0xd000, buf, 32) != DFW_OK) msg("err\n");
  if (dfwup_read_memory(0xc000, buf, 32) != DFW_OK) msg("err\n");
}

/* ----------------------------------------------------------- */

usage(char *name)
{
  printf("%s [-vdf] [-p /dev/cu.usbmodemXXX] [-c TARGET_OPTIONS] "
         "[-b name.txt from-addr] [object]\n\n", name);
  printf("  object can be elf, ihex, or titxt\n"
         "  /dev/cu.usbmodemXXX might be /dev/ttyACMXXX or /dev/ttyXXX in some system\n"
         "\n"
         "TARGET_OPTIONS: (See cmd.c target_*.c for details)\n\n"
         "   MSP430    : (default) msp430x1xx, msp430x2xx, msp430x4xx\n"
         "   CPUXV1    :           msp430x2xx, msp430x4xx\n"
         "   CPUXV2    :           msp430x5xx, cc430x6xx \n"
         "   CC8051    :           cc111x, cc251x, etc.  \n"
         "\n"
         "   VCC xxxx  : sets VCC to XXXX mV (e.g., VCC 3300), FET430UIF only.\n"
         "\n"
         "   JTAG      : (default)\n"
         "   SBW       :          \n"
         "\n"
         "   FastFlash : (default)\n"
         "   SlowFlash :           for relatively old devices without FastFlash feature\n"
         "   LOCKA     : (default) locks INFO_A memory\n"
         "   UNLOCKA   :           unlocks INFO_A memory\n"
         "\n"
         "   FCTL addr : sets FCTL base address for CPUXV2 target.\n"
         "\n"
         "   FWS  n    : sets flash word size for CC8051 target.\n"
         "\n"
         "URL http://shkmr.github.io/msp430/dfw/\n\n"
         );
  exit(1);
}

int main(int c, char *v[])
{
  int       libdfw_debug_level = 0;
  char      **matching;
  char      *objname, *options, *cmdname, *flags;
  char      *bkupstr;
  char      *fet;
  unsigned long bkupfrom = 0x1100;

  objname = NULL;
  options = "MSP430 JTAG VCC 3300";
  bkupstr = NULL;
  fet     = NULL;

  cmdname = v[0]; v++, c--;

  if (c == 0) usage(cmdname);

  while (c > 0) {
    if (v[0][0] == '-') {
      flags = &v[0][1];
      while (*flags) {
	switch (*flags) {
	case 'b'  :
	  v++, c--;
	  if (c > 0) bkupstr = v[0];
	  v++, c--;
	  if (c > 0) bkupfrom = strtol(v[0], NULL, 16);
	  break;
	case 'c'  :
	  v++, c--; 
	  if (c > 0) options = v[0]; 
	  break;
	case 'd'  : debug++;              break;
	case 'v'  : libdfw_debug_level++; break;
	case 'f'  : firmware++;           break;
        case 'p'  : 
	  v++, c--; 
          if (c > 0) fet = v[0];
          break;
	default:
	  usage(cmdname);
	}
	flags++;
      }
    } else {
      objname = v[0];
    }
    v++, c--;
  }
  libdfw_set_debug_level(libdfw_debug_level);
  atexit(cleanup);

  if (debug > 3) {
    fprintf(stderr, "%s: libdfw=%d, firmware=%d, obj=%s\n", 
	    cmdname,
	    libdfw_debug_level,
	    firmware,
	    objname);
    fprintf(stderr, "options=\"%s\"\n", options);
  }

  if (objname != NULL) obj_read(objname);

  libdfw_init(fet);

  if (bkupstr != NULL) backup(options, bkupstr, bkupfrom, 0xffff);
  
  if (objname != NULL)
    if (firmware)
      firmware_update();
    else 
      target_update(options);
  
  return 0;
}

/* EOF */
