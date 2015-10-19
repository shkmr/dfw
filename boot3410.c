/*
  TUSB3410 boot:

  Public Function

    boot3410() : send program image file to TUSB3410

  Reference

    Section 11.7 of ``TUSB3410/I Data Manual'' (SLL519D).
    SLLC139.zip is the source code for the device's ROM.

                         written by skimu@mac.om
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "usbcom.h"

const int BUF_SIZE  = 32768;

static uint8_t checkcum(uint8_t *buf, int len)
{
  uint8_t sum;

  sum = 0;
  while (len--)
    sum += *buf++;

  return sum;
}
  
static int send3410(uint8_t *buf, int size, 
                    int vendor, int product)
{
  int r = 0;
  usbcom_t com;

  com = usbcom_open(vendor, product);
  if (com == NULL)
    err(1, "open failed");

  if (usbcom_npipe(com) == 1) {
    r = usbcom_send(com, 1, buf, size);
    if (r)
      warnx("usbcom_send returned non zero value (%d, 0x%x)\n", r, r);

    usbcom_re_enumerate(com);

    sleep(1);  /* Wait for re-enumeration to take place */
  }

  usbcom_close(com);

  return r;
}

/* 
 * Public function: boot3410()
 *
 *   vendor  : vendor id.
 *   product : product id.
 *   fn      : path to binary file (ex. "umpf3410.i51")
 *
 *  Returns non-zero on error
 */
int boot3410(int vendor, int product, char *fn)
{
  FILE      *fp;
  uint8_t   *buf;
  int        size;
  int        r = 1;

  if ((buf = malloc(BUF_SIZE)) == NULL) err(1, NULL);
  if ((fp  = fopen(fn, "rb"))  == NULL) goto error;
      
  size = fread(buf + 3, 1, BUF_SIZE, fp);

  /* TUSB3410 program should be within 16KBytes */

  if ((size > 0) && (size < 16385)) { 
    uint8_t cs;

    cs = checkcum(buf + 3, size);

    buf[0] = (size & 0xff);
    buf[1] = ((size>>8) & 0xff);
    buf[2] = cs;

    r = send3410(buf, size + 3, vendor, product);

  }
 error:
  free(buf);
  return r;
}

#ifdef BOOT3410_STANDALONE

main(int c, char *v[])
{
  int vendor, product;
  char *fn;

  fn      = "firm.flat";  /* default name      */
  vendor  = 0x0451;       /* Texas Instruments */
  product = 0xf430;       /* FET430UIF         */

  switch (c) {
  case 2:
    fn = v[1];
    break;
  case 4:
    fn      = v[1];
    vendor  = strtol(v[2], NULL, 16);
    product = strtol(v[3], NULL, 16);
    break;
  default:
    errx(1, "usage %s file [vendor product]", v[0]);
  }

  return boot3410(vendor, product, fn);
}

#endif

/* EOF */
