/* ptlink.c : communication between host through TUSB3410, path throgh  */

#include <stdint.h>
#include <signal.h>
#include <io.h>
#include "ptlink.h"
#include "misc.h"

enum { 
 BUFSIZE   =  128
};

static uint8_t      buf0[BUFSIZE];
static uint8_t     *wp0;
static uint8_t     *rp0;
static uint8_t     *end0;
volatile int nc0;

static uint8_t      buf1[BUFSIZE];
static uint8_t     *wp1;
static uint8_t     *rp1;
static uint8_t     *end1;
volatile int nc1;

void init_ptlink()
{
  nc0  = 0;
  wp0  = rp0 = &buf0[0];
  end0 = buf0 + BUFSIZE;

  nc1  = 0;
  wp1  = rp1 = &buf1[0];
  end1 = buf1 + BUFSIZE;
}

inline int getchar0()
{
  int    c;

  // while (nc0 == 0) ;
  /*  nc0-- is complied to add #llo(-1), &nc0 */
  c = *rp0++; nc0--;
  if (rp0 == end0) rp0 = &buf0[0];

  return c;
}

inline int getchar1()
{
  int    c;

  // while (nc1 == 0) ;

  c = *rp1++; nc1--;
  if (rp1 == end1) rp1 = &buf1[0];

  return c;
}

interrupt (UART0RX_VECTOR) usart0_rx()
{
  *wp0++ = U0RXBUF; nc0++;

  if (nc0 > BUFSIZE) {
    wp0--;
    nc0--;
  }
  if (wp0 == end0)  wp0 = &buf0[0];

  //  U1IE |= UTXIE1;
}

interrupt (UART1RX_VECTOR) usart1_rx()
{
  *wp1++ = U1RXBUF; nc1++;

  if (nc1 > BUFSIZE) {
    wp1--;
    nc1--;
  }
  if (wp1 == end1)  wp1 = &buf1[0];

  //  U0IE |= UTXIE0;
}

interrupt (UART0TX_VECTOR) usart0_tx()
{
  if (nc1 > 0)
    U0TXBUF = getchar1();
  else
    U0IE &= ~UTXIE0;
}

interrupt (UART1TX_VECTOR) usart1_tx()
{
  if (nc0 > 0)
    U1TXBUF = getchar0();
  else
    U1IE &= ~UTXIE1;
}

/* EOF */
