/* Simple IntelHex to Binary translator			*
 * written by Bodo Rzany				*/

#include <stdlib.h>
#include <stdio.h>

extern int ihex(char *filename, unsigned char* buffer, int len);

int main(int argc, char *argv[]) {
  FILE *ofile;
  unsigned char *buf;		// image buffer
  int len = 32768;		// with 32768 bytes
  int hiads;
  
  if ( argc < 3 ) {
    printf("usage: ihex2flat <ihex-file> <flat-file>\n");
    return -1;
  }
  
  if ( (buf = malloc(len)) == NULL ) {
    printf("image buffer allocation failed\n");
    return -1;
  }
  
  hiads = ihex(argv[1], buf, len);
  printf("done with topads = 0x%x\n", hiads);
  if ( (ofile = fopen(argv[2], "w+")) == NULL ) {
    printf("Can't open/create flatfile\n");
    return -1;
  }
  fwrite(buf, hiads, 1, ofile);
  fclose(ofile);
  return 0;
}
