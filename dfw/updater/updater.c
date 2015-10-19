/*
  Firmware updater for dfw/FET430UIF 
            written by skimu@mac.com
*/

#ifdef TARGET
#include <io.h>
#include <signal.h>
#include <stdint.h>
#else
#include <stdio.h>
#include <stdint.h>
#endif

#ifdef TARGET
asm(".data                \n"
    "start:               \n"
    "mov #__stack, r1     \n"
    "br #updater          \n");
#else
main()
{
  updater();
}
#endif

/* ------------------------------------------------------------- */

#ifdef TARGET
#define ram __attribute__((section(".data")))
#else
#define ram
#endif

/* ------------------------------------------------------------- */

#define FLASH_START  0x2500 /* MSP430F1612 */

#define MCLK_FREQ      8000 /* MCLK in KHz */
#define FTG_FREQ        400 /* Flash timing generator in KHz */

#define RXBUF U0RXBUF
#define TXBUF U0TXBUF
#define isRXBUFready()  (U0IFG&URXIFG0)
#define isTXBUFready()  (U0IFG&UTXIFG0)

#define ERASE_SEGMENTS  2
#define ERASE_MAIN      4
#define ERASE_MAIN_INFO 6
#define ERASE_ALL       ERASE_MAIN_INFO


/* ------------------------------------------------------------- */

#ifdef TARGET
ram getchar()
{
  while (!isRXBUFready()) ;
  return RXBUF; 
}

ram putchar(int c)
{
  while (!isTXBUFready()) ;
  TXBUF = c;
}
#endif

/* ------------------------------------------------------------- */

ram putstr(char *str)
{ 
  int c;
  while (c=*str++) putchar(c);
}

/* ------------------------------------------------------------- */

ram CTOH(x) {return x >'9' ?  x - 'A' + 0x0A : x - '0';}
ram HTOC(x) {return x > 9  ?  x - 0x0A + 'A' : x + '0';}

ram isxdigit(int c)
{
  return ((c >= '0')&&(c <= '9')) || ((c >= 'A') && (c <= 'F'));
}

ram gethex8()
{
  int x, y;
  if (!isxdigit(x=getchar())) return -1;
  if (!isxdigit(y=getchar())) return -1;
  return (CTOH(x)<<4) + CTOH(y);
}

ram gethex16()
{
  int x = gethex8();
  return (x<<8) + gethex8();
}

ram puthex(int h)
{
  int x, y;
  x = ((h>>4)&0x0f);
  y = (h&0x0f);
  putchar(HTOC(x));
  putchar(HTOC(y));
}

ram wait_for_nl() { while (getchar() != '\n') ; }

/* ------------------------------------------------------------- */

#define flash_wait()           {while (FCTL3&BUSY);}
#define flash_unlock()         (FCTL3=FWKEY)
#define flash_lock()           (FCTL3=(FWKEY|LOCK))
#define flash_erase_mode(mode) (FCTL1=(FWKEY|mode))
#define flash_write_mode()     (FCTL1=(FWKEY|WRT))
#define flash_nop_mode()       (FCTL1=FWKEY)

ram erase_flash(int mode, int *addr)
{
#ifdef TARGET
  /* mode 0 no erase
          2 segment
	  4 all main segments
	  6 all main segments and info */
  if (mode > 6) return;

  flash_unlock();
  flash_erase_mode(mode);
  *addr = 0;
  flash_wait();
  flash_nop_mode();
#else
  printf("erase_flash mode=%d addr=0x%04x\n", mode, (int)addr);
#endif
}

ram write_flash(int *from, int *to, int len)
{
#ifdef TARGET
  flash_unlock();
  flash_write_mode();
  while (len-- > 0) { *to++ = *from++; flash_wait();}
  flash_lock();
  flash_nop_mode();
#else
  printf("write_flash to=0x%04x len=0x%04x\n", (int)to, (int)len);
#endif
}

/* ------------------------------------------------------------- */

ram hexwrite_line(uint8_t *addr, int len)
{
  uint8_t    checksum;
  int        i, c;

#define puth(x) {c=x; puthex(c); checksum -= c;}
  checksum = 0;
  putchar(':');
  puth(0);
  puth(len);
  puth((int)addr>>8);
  puth((int)addr)
  for (i=0; i < len; i++) {
#ifdef TARGET
    puth(*addr++);
#else
    puth((addr++,0));
#endif
  }
  puth(checksum);
  putchar('\n');
  wait_for_nl();
}

#define HEXWRITE_MAX_BYTES 32

#ifdef TARGET
extern char endmarker[];
asm(".global endmarker       \n"
    "endmarker:              \n"
    ".string \":01000000FF\\n\"");
#else
char endmarker[] = ":01000000FF\n";
#endif

ram hexwrite(uint8_t *addr, uint16_t len)
{
  while (len > HEXWRITE_MAX_BYTES) {
    hexwrite_line(addr, HEXWRITE_MAX_BYTES);
    addr += HEXWRITE_MAX_BYTES;
    len  -= HEXWRITE_MAX_BYTES;
  }
  hexwrite_line(addr, len);
  putstr(endmarker);
}

/* ------------------------------------------------------------- */

ram read_memory()
{
  uint16_t    addr, len;

  getchar();
  addr = gethex16();
  getchar();
  len  = gethex16();
  wait_for_nl();
  hexwrite((uint8_t *)addr, len);
  return 0;
}

/* ------------------------------------------------------------- */

#define HEXLEN_MAX 64  /* maximum acceptable bytes in one line */
ram error_occured()  {wait_for_nl(); putchar('N'); putchar('\n');}
ram ready_for_data() {wait_for_nl(); putchar('K'); putchar('\n');}

ram write_memory()
{
  uint8_t    buf[HEXLEN_MAX], *p;
  uint8_t    type, len, checksum;
  uint16_t   addr;
  int        i;

  while (1) {
    ready_for_data();
    if (getchar() != ':') goto error;
    type = gethex8();
    if (type > 1)         goto error;
    len  = gethex8();
    if (len > HEXLEN_MAX) goto error;
    addr = gethex16();
    checksum =  type;
    checksum += len;
    checksum += (addr + (addr >> 8));

    for (p = buf, i=0; i < len; p++, i++) {
      *p = gethex8(); checksum += *p;
    }
    if (len%2) *p = 0xff;   /* maximum odd len is (HEXLEN_MAX-1) */

    checksum += gethex8();
    if (checksum) goto error;

    switch (type) {
    case 0:
      if (addr >= FLASH_START) {
	write_flash((int16_t *)buf, (int16_t *)addr, len/2 + len%2);
	break;
      } else {
	goto error;
      }
    case 1: return;
    default:
      goto error;
    }
  }
 error:
  error_occured();
}

/* ------------------------------------------------------------- */

#define FTG_DIV  (MCLK_FREQ/FTG_FREQ)

ram go_PUC()
{
  if ((getchar()=='U')&&(getchar()=='C')) {
    wait_for_nl();
    /* P3.6 : RST3410 */
    /* P3OUT &= ~BIT6;
       P3DIR |=BIT6;  */
    WDTCTL=0;
  } else
    wait_for_nl();
}

#define power_LED_on()   (P1OUT|= BIT1)
#define power_LED_off()  (P1OUT&=~BIT1)
#define power_LED_flip() (P1OUT^= BIT1)

ram updater()
{
#ifdef TARGET
  dint();
  FCTL2  = (FWKEY|FSSEL_MCLK|FTG_DIV);
#endif

  putchar('b'); putchar('\n');
  while (1) {
    switch (getchar()) {
    case '\n': putchar('*'); putchar('\n'); break;
    case 'e' : erase_flash(ERASE_ALL, (int16_t *)0xFFFF); break;
    case 'f' : write_memory(); break;
    case 'r' : read_memory();  break;
    case 'P' : go_PUC();
    default: ;
    }
  }
}

/* EOF */
