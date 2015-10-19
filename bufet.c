/*
 *     bufet.c : backup/restore FET430UIF/eZ430U.
 *
 *     XXX little endian host computer assumed.
 *     See, write_uint16() how to fix.
 *
 *                      written by skimu@mac.com
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <err.h>
#include <sys/time.h>
#include <assert.h>
#include "dfw/libdfw.h"
#include "segment.h"

usage(char *name)
{
  printf("%s [-vd] [-p /dev/cu.usbmodemXXX] [-c TARGET_OPTIONS] [fet_eeprom.txt fet_flash.txt]\n", name);
  printf("Make backup files for EEPROM and FLASH memory of FET430UIF/eZ430UIF,\n"
         "or restores those files if both fet_eeprom.txt and fet_flash.txt are specified.\n"
         "In backup mode, fet_eeprom.txt and fet_flash.txt will be craeted.\n\n"
         "/dev/cu.usbmodemXXX might be /dev/ttyACMXXX or /dev/ttyXXX in some system\n\n"
         "TARGET_OPTIONS:\n\n"
         "   VCC xxxx  : sets VCC to XXXX mV (e.g., VCC 3300), FET430UIF only.\n\n"
         "   JTAG      : (default)\n\n"
         "URL http://homepage.mac.com/skimu/msp430/dfw/\n\n"
         );
  exit(1);
}

/*----------------------------------------------------------------------- */

int debug  = 0;

void cleanup()
{
  libdfw_finish();
}

/*--------------------- hexdump, in TITXT format ------------------------ */

hexdump1(FILE *fp, int addr, uint8_t *data, int len)
{
  while (len-- > 0)
    fprintf(fp, "%02X ", *data++);
  fprintf(fp, "\n");
}

hexdump(FILE *fp, int addr, uint8_t *data, int len)
{
  const int   ncol = 16;

  fprintf(fp, "@%04X\n", addr);
  while (len > ncol) {
    hexdump1(fp, addr, data, ncol);
    addr += ncol;
    data += ncol;
    len  -= ncol;
  }
  hexdump1(fp, addr, data, len);
}

/* -------------------------- backup ------------------------------------ */

const char bufetee[] = {
#include "bufetee/bufetee_object_array.inc"
};

const char firm[] = { /* for debug verification */
#include "tusb3410_autoexec_firmware.inc"
};

enum MSP430F1612 {
  RAM_START     = 0x1100,
  FLASH_START   = 0x2500,
};

enum {
  EEB_SIZE      = 512,
};

uint8_t   eeb[EEB_SIZE];

write_uint16(unsigned addr, uint16_t val)
{
  uint16_t    temp;

  temp = val;
  if (0/* host is big endian*/)
    dfw_swap_uint16(&temp, 1);
  dfw_target_write_memory(addr, (uint8_t *)&temp,  2);
}

backup_eeprom(FILE *fp)
{
  unsigned   addr;
  unsigned   len  = EEB_SIZE;
  unsigned   buf  = RAM_START + sizeof(bufetee) + 32; // reserve 32 bytes for .bss section, just in case
  int        j;

  memset(eeb, 0, sizeof(eeb));

  dfw_target_write_memory(RAM_START, bufetee, sizeof(bufetee));
  dfw_target_write_memory(buf, eeb, sizeof(eeb));

  for (addr = 0; addr < 16384; addr += EEB_SIZE) {

    write_uint16(RAM_START+0x10, addr);
    write_uint16(RAM_START+0x12, buf);
    write_uint16(RAM_START+0x14, len);

    dfw_target_write_register(2, 0);         // set SR = 0
    dfw_target_write_register(1, 0x2500);    // set SP = 0x2500
    dfw_target_write_register(0, RAM_START); // set PC = 0x1100
    dfw_target_continue_cpu();
    j = 0;
    while (!dfw_target_is_cpu_stopped()) {
      if (j++ > 10)
        errx(1, "something wrong");
      usleep(300000);
    }
    dfw_target_read_memory(buf, eeb, len);
    hexdump(fp, addr, eeb, len);
    fprintf(stderr, ".");
  }
  fprintf(stderr, "\n");
  fprintf(fp, "q\n");
}

backup_flash(FILE *fp)
{
  unsigned   addr;
  unsigned   len;

  addr = FLASH_START;
  len  = 0x10000 - FLASH_START;

  while (len > EEB_SIZE) {
    fprintf(stderr, ".");
    dfw_target_read_memory(addr, eeb, EEB_SIZE);
    hexdump(fp, addr, eeb, EEB_SIZE);
    addr += EEB_SIZE;
    len  -= EEB_SIZE;
  }
  fprintf(stderr, ".");

  dfw_target_read_memory(addr, eeb, len);
  hexdump(fp, addr, eeb, len);

  fprintf(stderr, "\n");
  fprintf(fp, "q\n");
}

backup(char *options)
{
  FILE    *ee, *fl;
  char    *eefn = "fet_eeprom.txt";
  char    *flfn = "fet_flash.txt";

  if ((ee = fopen(eefn, "w")) == NULL) err(1, NULL);
  if ((fl = fopen(flfn, "w")) == NULL) err(1, NULL);

  dfw_open_target(options);
  fprintf(stderr,"backing up eeprom into %s", eefn);
  backup_eeprom(ee);
  fprintf(stderr,"backing up flash into %s ", flfn);
  backup_flash(fl);
  dfw_close_target();

  fclose(ee);
  fclose(fl);
}

/* ------------------------ TITXT reader -------------------------------- */

uint8_t  txt_buf[65536];
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
  if (txt_count > EEB_SIZE)
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

/* ------------------------ restore ------------------------------------- */

enum MICROCHIP_24LC128 {
  EEPROM_PAGE_SIZE  = 64
};

write_ee1(unsigned addr, uint8_t *buf, unsigned len)
{
  unsigned temp = RAM_START + sizeof(bufetee) + 32;
  int      j;

  assert(len <= EEPROM_PAGE_SIZE);

  write_uint16(RAM_START+0x10, addr);
  write_uint16(RAM_START+0x12, temp);
  write_uint16(RAM_START+0x14, len);

  dfw_target_write_memory(temp, buf, len);

  dfw_target_write_register(2, 0);           // set SR = 0
  dfw_target_write_register(1, 0x2500);      // set SP = 0x2500
  dfw_target_write_register(0, RAM_START+8); // set PC = 0x1108, entry point for restore
  dfw_target_continue_cpu();
  j = 0;
  while (!dfw_target_is_cpu_stopped()) {
    if (j++ > 10)
      errx(1, "something wrong");
    usleep(300000);
  }
}

write_ee(unsigned addr, uint8_t *buf, unsigned len)
{
  while (len > EEPROM_PAGE_SIZE) {
    write_ee1(addr, buf, EEPROM_PAGE_SIZE);
    addr += EEPROM_PAGE_SIZE;
    buf  += EEPROM_PAGE_SIZE;
    len  -= EEPROM_PAGE_SIZE;
  }
  write_ee1(addr, buf, len);
}

restore_eeprom()
{
  segment_t   *seg;

  dfw_target_write_memory(RAM_START, bufetee, sizeof(bufetee));

  for (seg = segment_start(); seg != NULL; seg = segment_next(seg)) {
    fprintf(stderr, ".");
    write_ee(segment_addr(seg),
             segment_buf(seg),
             segment_size(seg));
  }
  fprintf(stderr, "\n");
}

restore_flash()
{
  segment_t   *seg;

  for (seg = segment_start(); seg != NULL; seg = segment_next(seg)) {
    fprintf(stderr, ".");
    dfw_target_write_flash(segment_addr(seg),
                           segment_buf(seg),
                           segment_size(seg));
  }
  fprintf(stderr, "\n");
}

restore(char *options, char *eefn, char *flfn)
{
  dfw_open_target(options);

  fprintf(stderr,"restoring eeprom from %s ", eefn);
  segment_free();
  txt_read(eefn);
  restore_eeprom();

  fprintf(stderr,"mass erasing flash...");
  dfw_target_erase_flash(ERASE_MASS, 0xfff0);
  fprintf(stderr,"done.\n");

  fprintf(stderr,"restoring flash from %s ", flfn);
  segment_free();
  txt_read(flfn);
  restore_flash();

  dfw_close_target();
}

/* ---------------------------------------------------------------------- */

main(int c, char *v[])
{
  int       libdfw_debug_level = 0;
  char      *cmdname, *fet, *options, *flags;
  char      *eefn, *flfn;

  eefn    = NULL;
  flfn    = NULL;
  fet     = NULL;
  options = "MSP430 JTAG VCC 3300";

  cmdname = v[0]; v++, c--;

  // if (c == 0) usage(cmdname);

  while (c > 0) {
    if (v[0][0] == '-') {
      flags = &v[0][1];
      while (*flags) {
	switch (*flags) {
	case 'c'  :
	  v++, c--; 
	  if (c > 0) options = v[0]; 
	  break;
	case 'd'  : debug++;              break;
	case 'v'  : libdfw_debug_level++; break;
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
      eefn = v[0], v++, c--;
      flfn = v[0];
    }
    v++, c--;
  }
  libdfw_set_debug_level(libdfw_debug_level);
  atexit(cleanup);
  libdfw_init(fet);

  if (eefn && flfn)
    restore(options, eefn, flfn);
  else if (eefn == NULL && flfn == NULL)
    backup(options);
  else
    usage(cmdname);

  return 0;
}

#if 0 /* DEBUG VERIFY */
  if (debug > 3) {
      if (addr + len < sizeof(firm)) {
        if (memcmp(firm+addr, eeb, len)) 
          errx(1, "veryfy error");
        else
          fprintf(stderr, "O");
      } else if (addr < sizeof(firm)) {
        if (memcmp(firm+addr, eeb, sizeof(firm)-addr))
          errx(1, "veryfy error");
        else
          fprintf(stderr, "O");
      } else
        fprintf(stderr, ".");
    } else
        fprintf(stderr, ".");
#endif

/* EOF */
