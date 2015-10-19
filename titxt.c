/*
 * READ TI TXT
 *
 * Public function
 *   titxt_read() : see below example for usage.
 *
 *                         written by skimu@mac.com
 */

#include <stdio.h>

static int char_num(int c)
{
  /* Assuming ASCII family */

  switch (c) {
  case '0':  case '1':  case '2':  case '3':  case '4':
  case '5':  case '6':  case '7':  case '8':  case '9':
    return c - '0';
  case 'A':  case 'B':  case 'C':
  case 'D':  case 'E':  case 'F':
    return c - 'A' + 10;
  case 'a':  case 'b':  case 'c':
  case 'd':  case 'e':  case 'f':
    return c - 'a' + 10;
  default:
    return 0;
  }
}

static int read_hexnum(FILE *fp, int l)
{
  int x, c;

  x = char_num(l);

  while (1) {
    c = getc(fp);
    if (c == EOF) {
      return x;
    }
    if (isspace(c)) {
      ungetc(c, fp);
      return x;
    }
    x *= 16;
    x += char_num(c);
  }
}

void titxt_read(FILE *fp, void (*set_addr)(int), void (*put_byte)(int))
{
  int c;
  while ((c = getc(fp)) != EOF) {
    if (isspace(c))
      continue;
    switch (c) {
    case '@':
      set_addr(read_hexnum(fp, '0'));
      break;
    case 'Q':
    case 'q':
      return;
    default:
      put_byte(read_hexnum(fp, c));
      break;
    }
  }
}

#ifdef TITXT_STANDALONE

/*
 *     An example how to use titxt_read().
 */

#include "hex.h"

static unsigned char data[16];
static int  addr;
static int  count;

static void start_data()
{
  addr = 0;
  count = 0;
  hex_write_begin(stdout);
}

static void finish_data()
{
  hex_write(stdout, addr, count, data);
  hex_write_end(stdout);
}

static void set_addr(int x)
{
  hex_write(stdout, addr, count, data);
  addr = x;
  count = 0;
}

static void put_byte(int x)
{
  data[count++] = x;
  if (count >= 16) {
    hex_write(stdout, addr, 16, data);
    addr += 16;
    count = 0;
  }
}

int main(int c, char *v[])
{
  start_data();
  titxt_read(stdin, set_addr, put_byte);
  finish_data();
  return 0;
}

#endif
/* EOF */
