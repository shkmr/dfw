/* datalink.c :  USCI */

#include <stdint.h>
#include <signal.h>
#include <io.h>

#include "uif.h"
#include "datalink.h"
#include "misc.h"

#define BUFSIZE   64

extern int        debug;

static uint8_t    buf[BUFSIZE];
static uint8_t    *wp;
static uint8_t    *rp;
static uint8_t    *end;
static volatile   int nc;

void init_datalink()
{
  nc = 0;
  wp = rp = &buf[0];
  end = buf + BUFSIZE;
}

int putchar(int c)
{
  while ((IFG2&UCA0TXIFG) == 0) ;
  UCA0TXBUF = c;
  return c;
}

void putstr(char *s)
{
  while (*s) putchar(*s++);
}

int getchar()
{
  int    c;

  while (nc == 0);
  c = *rp++; nc--;
  if (rp == end) rp = &buf[0];
  return c;
}

int peekchar()
{
  while (nc == 0) ;
  return *rp;
}

void gettoken(char *s, int n)
{
  int    i;

  n--;
  while (peekchar() == ' ') getchar();
  for (i=0; i < n; i++) {
    switch (peekchar()) {
    case ' '  :
    case '\n' : goto finish;
    default:
      *s++ = getchar();
    }
  }
 finish:
  *s = '\0';
  return;
}

interrupt (USCIAB0RX_VECTOR) usart0_rx()
{
  *wp++ = UCA0RXBUF; nc++;
  if (wp == end) wp = &buf[0];
  if (nc > BUFSIZE) panic();
}

/* EOF */
