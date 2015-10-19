/* 
   remove all FF lines from TITXT 
                    written by skimu@mac.com
*/

#include <stdio.h>

extern void titxt_read();

static unsigned char data[16];
static int  addr;
static int  count;
static int  skipped;

hex_write(unsigned char *buf, int len)
{
  while (len-- > 0) printf("%02X ", *buf++);
  printf("\n");
}

void set_addr(int x)
{
  printf("@%04X\n", x);
  addr = x;
  count = 0;
}

int is_all_ff(unsigned char *buf, int len)
{
  while (len-- > 0)
    if (*buf++ != 0xff)
      return 0;
  return 1;
}

void put_byte(int x)
{
  data[count++] = x;
  if (count >= 16) {
    if (is_all_ff(data, 16)) {
      skipped = 1;
    } else {
      if (skipped) set_addr(addr);
      hex_write(data, 16);
      skipped = 0;
    }
    addr += 16;
    count = 0;
  }
}

void flush_rest()
{
  if (count > 0 && !is_all_ff(data, count)) {
    if (skipped) set_addr(addr);
    hex_write(data, count);
  }
}

int main(int c, char *v[])
{
  addr     = 0;
  count    = 0;
  skipped  = 0;
  titxt_read(stdin, set_addr, put_byte);
  flush_rest();
  printf("q\n");
  return 0;
}

/* EOF */
