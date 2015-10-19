#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>

const int MAXCOL = 8;
const int MAXBUF = 65536;

dump1(uint8_t *buf, int size)
{
  printf("   ");
  while (size-- > 0) {
    printf(" 0x%02x,", *buf++);
  }
  printf("\n");
}

dump(uint8_t *buf, int size)
{
  printf("/* Firmware updater binary, %d bytes */\n", size);
  printf("/* uint8_t updater[] = { */\n");
  while (size > MAXCOL) {
    dump1(buf, MAXCOL);
    size -= MAXCOL;
    buf  += MAXCOL;
  }
  dump1(buf, size);
  printf("/* }; */\n/* EOF */\n");
}

ext(char *f)
{
  FILE    *fp;
  int     size;
  uint8_t buf[MAXBUF];

  if ((fp = fopen(f, "rb")) == NULL) err(1, f);
  
  size = fread(buf, 1, MAXBUF, fp);
  dump(buf, size);
  fprintf(stderr, "%d bytes\n", size);

  fclose(fp);
  return 0;
}

main(int c, char *v[])
{
  if (c > 1) 
    return ext(v[1]);
  else
    errx(1, "file to extract");
}

/* EOF */
