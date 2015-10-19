#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>

#include "libdfw.h"
	     
static int debug = 0;

/* --------------------------------------------------------- */
#if 0  /* MacOSX using usbcom. Support dropped over CDC ACM. */

#include "usbcom.h"

usbcom_t com;

#ifndef TUSB3410FIRM
#define TUSB3410FIRM "/usr/local/msp430/fet430uif/tusb3410-beef.bin"
#endif
int boot3410(int vendor, int product, char *fname);

open_fet430uif(char *fet)
{
  int    vendor, product;

  vendor  = 0x0451;   /* Texas Instruments */
  product = 0xbeef;   /* Taken from slaa276 */

  com = usbcom_open(vendor, product);
  if (com == NULL)
    errx(1, "USB open failed 0x%04x/0x%04x", vendor, product);
  
  if (usbcom_npipe(com) != 3) {

    /*
     * We need 3 pipes to deal with FET430
     * Put TUSB3410 firmware and retry     
     *
     */
    usbcom_close(com);

    if (debug)
      fprintf(stderr, "init: loading TUSB3410 firmware\n");

    boot3410(vendor, product, TUSB3410FIRM);

    com = usbcom_open(vendor, product); 

    if ((com == NULL) || (usbcom_npipe(com) != 3))
      errx(1, "USB open failed 0x%04x/0x%04x", vendor, product);
  }
  return;
}

close_fet430uif()
{
  usbcom_close(com);
}

static int dfwsend(uint8_t *buf, int len)
{
  return usbcom_send(com, 1, buf, len);
}

static int dfwreceive(uint8_t *buf, int len)
{
  return usbcom_receive(com, 2, buf, len);
}

#else

#include <fcntl.h>

#ifndef DEFAULT_TTY
#ifdef  __APPLE__
#define DEFAULT_TTY   "/dev/cu.usbmodem001"
#else  /* __linux__ */
#define DEFAULT_TTY   "/dev/ttyACM0"
#endif
#endif /* !DEFAULT_TTY */

static int fetio;

open_fet430uif(char *fet) 
{ 
  struct termios tio;

  if (fet == NULL)
    fet = DEFAULT_TTY;
  if ((fetio = open(fet, O_RDWR)) < 0) 
    err(1, fet);

  tcgetattr(fetio, &tio);
  cfmakeraw(&tio);
  if (tcsetattr(fetio, TCSANOW, &tio) < 0) 
    err(1, NULL);

  return 0;
}

close_fet430uif()
{
  close(fetio);
}

static int dfwsend(uint8_t *buf, int len)
{
  return write(fetio, buf, len);
}

static int dfwreceive(uint8_t *buf, int len)
{
  int    n, total;
  
  n = total = 0;
  while (len > 0) {
    n = read(fetio, buf, len);
    total += n;
    if (buf[n-1] == '\n') break;
    buf += n;
    len -= n;
  }
  return total;
}
#endif

/* ------------------------------------------------------ */

#define LINE_CHAR_MAX 127

static int dfwputs(char *str)
{
  if (debug) printf("T:%s", str);
  dfwsend(str, strlen(str));
}

static int dfwgets(char *buf)
{
  int    n;
  char   bb[LINE_CHAR_MAX+1];

  while ((n=dfwreceive(bb, LINE_CHAR_MAX)) == 0) ;

  bb[n] = '\0';
  if (debug) printf("R:%s", bb, n);
  strncpy(buf, bb, LINE_CHAR_MAX);
}

/* ------------------------------------------------------ */

CTOH(x) {return x >'9' ?  x - 'A' + 0x0A : x - '0';}
HTOC(x) {return x > 9  ?  x - 0x0A + 'A' : x + '0';}

static int isxdigit(int c)
{
  return ((c >= '0')&&(c <= '9')) || ((c >= 'A') && (c <= 'F'));
}

static int gethex8(char *buf)
{
  int    x, y;

  if (!isxdigit(x=buf[0])) return -1;
  if (!isxdigit(y=buf[1])) return -1;
  return (CTOH(x)<<4) + CTOH(y);
}

static int gethex16(char *buf)
{
  int    x = gethex8(buf);

  return  (x<<8) + gethex8(buf+2);
}

static int hexwrite_line(uint32_t addr, const uint8_t *src, int len)
{
  char    buf[LINE_CHAR_MAX];
  char    *p;
  uint8_t checksum;
  int     i, c;

  p = buf;
  *p++ = ':';
  checksum = 0;
#define puth(x) {c=x; *p++ = HTOC((c>>4)&0x0f); *p++ = HTOC(c&0x0f); checksum -= c;}
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
  for (i=0; i < len; i++, src++) puth(*src);
  puth(checksum);
  *p++ = '\n';
  *p++ = '\0';
  dfwputs(buf);
  dfwgets(buf);
  return buf[0] == 'K' ? DFW_OK : DFW_ERROR;
#undef puth
}

static int hexread(uint8_t *dst)
{
  char      buf[LINE_CHAR_MAX];
  char      *p;
  uint32_t  addr;
  uint8_t   len, type, checksum;
  int       i, a, bp;

  bp = 0;

  while (1) {
    dfwgets(buf); p = buf; 

    checksum = 0;
    if (*p++ != ':') goto error;

#define geth(x) {x = gethex8(p); p += 2; checksum += x;}
    geth(type);
    geth(len);
    geth(a); addr = a;
    geth(a); addr = (addr<<8) + a;
    if (type == 8) {
      geth(a); addr = (addr<<8) + a;
      geth(a); addr = (addr<<8) + a;
    }
    /* (/ (- 127 1 1 1 8 1 1) 2)57 */
    if (len > 50) goto error;
    for (i=0; i < len; i++, bp++) geth(dst[bp]);
#undef geth

    checksum += gethex8(p);
    if (checksum)   goto error;
    if (type == 1)  return DFW_OK;
    dfwputs("K\n");
  }

error:
  do {
    dfwputs("N\n");
    dfwgets(buf);
  } while (buf[0] != ':');
  return DFW_ERROR;
}

/* ------------------------------------------------------ */

static void sync_fet()
{
  char    buf[LINE_CHAR_MAX];
  do {
    dfwputs("z\n");
    dfwgets(buf);
  } while (buf[0] != '*') ;
}

void libdfw_set_debug_level(int n)
{
  debug = n;
}

void libdfw_init(char *fet)
{
  open_fet430uif(fet);
  sync_fet();
}

void libdfw_finish()
{
  close_fet430uif();
}

int dfw_open_target(const char *options)
{
  char    buf[LINE_CHAR_MAX];

  sync_fet();

  if (options != NULL) {
    snprintf(buf, LINE_CHAR_MAX, "c %s\n", options);
    dfwputs(buf);
  } else {
    dfwputs("c\n");
  }
  dfwgets(buf);
  if (buf[0] == 'K')
    return DFW_OK;
  else
    return DFW_ERROR;
}

int dfw_close_target()
{
  char    buf[LINE_CHAR_MAX];

  dfwputs("d\n");
  dfwgets(buf);
  if (buf[0] == 'K')
    return DFW_OK;
  else
    return DFW_ERROR;
}

int dfw_enter_updater()
{
  char    buf[LINE_CHAR_MAX];

  dfwputs("b\n");
  dfwgets(buf);
  if (buf[0] == 'b') 
    return DFW_OK;
  else 
    return DFW_ERROR;
}

int dfw_target_erase_flash(const char *mode, uint32_t addr)
{
  char    buf[LINE_CHAR_MAX];

  if (mode != NULL) {
    snprintf(buf, LINE_CHAR_MAX, "e %s %X\n", mode, addr);
    dfwputs(buf);
    dfwgets(buf);
    return DFW_OK;
  } else
    return DFW_ERROR;
}

const int HEXWRITE_MAX = 48;

static int target_write(uint32_t addr, const uint8_t *src, uint32_t len)
{
  char    buf[LINE_CHAR_MAX];

  dfwgets(buf);
  if (buf[0] != 'K') return DFW_ERROR;
  if (addr%2 && len > 1) {
    hexwrite_line(addr, src, 1);
    addr++, len--, src++;
  }
  while (len > HEXWRITE_MAX) {
    hexwrite_line(addr, src, HEXWRITE_MAX);
    addr += HEXWRITE_MAX;
    src  += HEXWRITE_MAX;
    len  -= HEXWRITE_MAX;
  }
  hexwrite_line(addr, src, len);
  dfwputs(":01000000FF\n");
  dfwgets(buf);
  return (buf[0] == '*' || buf[0] == '$') ? DFW_OK : DFW_ERROR;
}

int dfw_target_write_memory(uint32_t addr, const uint8_t *src, uint32_t len)
{
  dfwputs("s\n");
  return target_write(addr, src, len);
}

int dfw_target_write_flash(uint32_t addr, const uint8_t *src, uint32_t len)
{
  dfwputs("f\n");
  return target_write(addr, src, len);
}

int dfw_target_read_memory(uint32_t addr, uint8_t *dst, uint32_t len)
{
  char    cmd[LINE_CHAR_MAX];

  snprintf(cmd, LINE_CHAR_MAX, "r %04X %04X\n", addr, len);
  dfwputs(cmd);
  return hexread(dst);
}

/* ------------------------------------------------------ */

#if 0
int dfw_eeprom_read(uint32_t addr, uint8_t *dst, uint32_t len)
{
  char    cmd[LINE_CHAR_MAX];
  snprintf(cmd, LINE_CHAR_MAX, "u %X %X\n", addr, len);
  dfwputs(cmd);
  return hexread(dst);
}

int dfw_eeprom_write(uint32_t addr, uint8_t *src, uint32_t len)
{
  dfwputs("v\n");
  return target_write(addr, src, len);
}
#endif

/* ------------------------------------------------------ */
int dfw_target_is_cpu_stopped()
{
  char     buf[LINE_CHAR_MAX];

  dfwputs("j\n");
  dfwgets(buf);
  return !strncmp("STOPPED", buf, 7);
}

int dfw_target_stop_cpu()
{
  char     buf[LINE_CHAR_MAX];

  dfwputs("k\n");
  dfwgets(buf);
  return DFW_OK;
}

int dfw_target_continue_cpu()
{
  char     buf[LINE_CHAR_MAX];

  dfwputs("l\n");
  dfwgets(buf);
  return DFW_OK;
}

int dfw_target_step_cpu(int n)
{
  char     buf[LINE_CHAR_MAX];

  snprintf(buf, LINE_CHAR_MAX, "m %d\n", n);
  dfwputs(buf);
  dfwgets(buf);
  return DFW_OK;
}

void dfw_swab_uint16(void *buf, unsigned int len)
{
  int       i;
  uint8_t   tmp;
  uint8_t   *b = buf;

  for (i=0; i < len; i += 2) {
    tmp    = b[i];
    b[i]   = b[i+1];
    b[i+1] = tmp;
  }
}

void dfw_swab_uint32(void *buf, unsigned int len)
{
  int       i;
  uint8_t   t0, t1, t2, t3;
  uint8_t   *b = buf;

  for (i=0; i < len; i += 4) {
    t0 = b[i+0];
    t1 = b[i+1];
    t2 = b[i+2];
    t3 = b[i+3];
    b[i+0] = t3;
    b[i+1] = t2;
    b[i+2] = t1;
    b[i+3] = t0;
  }
}

int dfw_target_read_registers(uint8_t *reg, uint32_t len)
{
  char    buf[LINE_CHAR_MAX];
  int     r;

  if (len < 64) return DFW_ERROR;

  dfwputs("p\n");
  r = hexread(reg);
  return r;
}

int dfw_target_write_register(int n, uint32_t val)
{
  char      buf[LINE_CHAR_MAX];

  snprintf(buf, LINE_CHAR_MAX, "q %d %08x\n", n, val);
  dfwputs(buf);
  dfwgets(buf);
  return DFW_OK;
}

/* ------------------------------------------------------ */

int dfwup_PUC()
{
  dfwputs("PUC\n");
}

int dfwup_read_memory(uint32_t addr, uint8_t *dst, uint32_t len)
{
  char    cmd[LINE_CHAR_MAX];

  snprintf(cmd, LINE_CHAR_MAX, "r %04X %04X\n", addr, len);
  dfwputs(cmd);
  return hexread(dst);
}

int dfwup_erase_flash()
{
  char    buf[LINE_CHAR_MAX];

  dfwputs("e\n");
  dfwgets(buf);
  return DFW_OK;
}

/* EOF */
