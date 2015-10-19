/* 
 * USB com using libusb
 *
 * DEBUG ME:
 * I tried with libusb-0.1.12 on MacOSX, but did not work well.
 * It could successfully send tusb3410 firmware, but I do not know
 * how to re-enumerate on the fly.  After sending tusb3410's firmware, 
 * it seemed that it could send command to FET/DFW, but usb_bulk_read() 
 * returns error, could never get any reply form FET/DFW.
 *
 * In the end, we need new firmware for tusb3410, which can be 
 * recognized as tty device in both MacOSX and other Unix
 * and get rid of libusbcom.
 *
 */
#ifdef LIBUSB
#include <stdio.h>
#include <err.h>
#include <usb.h>
#include "usbcom.h"

struct usbcom {
  usb_dev_handle                  *dev;
  int                              ino;
  int                            npipe;
};

static int debug = 0;
usbcom_set_debug_level(int x) {debug = x;}

usbcom_t usbcom_open(int vendor, int product)
{
  struct usb_bus                   *bus;
  struct usb_device                *dev;
  struct usb_config_descriptor    *conf;
  struct usb_interface            *intf;
  struct usb_interface_descriptor  *ifd;
  int                          i, npipe;
  usbcom_t                          com;

  if ((com = (usbcom_t)malloc(sizeof(struct usbcom)))  == NULL) err(1, NULL);

  usb_init();
  usb_find_busses();
  usb_find_devices();

  for (bus = usb_get_busses(); bus; bus = bus->next) {
    for (dev = bus->devices; dev; dev = dev->next) {
      if (dev->descriptor.idVendor  == vendor  &&
          dev->descriptor.idProduct == product) {

        if (debug) fprintf(stderr, "usbcom: found device\n");

        conf        =   &dev->config[0];
        i           =   conf->bNumInterfaces - 1;  /* XXX the last interface : hard coded */
        intf        =   &conf->interface[i];
        ifd         =   &intf->altsetting[0];
        com->ino    =   ifd->bInterfaceNumber;
        com->npipe  =   ifd->bNumEndpoints;

        if (debug)
          fprintf(stderr, "bNumInterface=%d ino=%d npipe=%d\n", i, com->ino, com->npipe);

        if ((com->dev = usb_open(dev)) == NULL) {
          if (debug) fprintf(stderr, "usbcom: usb_open error\n");
          free(com);
          return NULL;
        }
        if (usb_claim_interface(com->dev, com->ino) < 0) {
          if (debug) fprintf(stderr, "usbcom: usb_claim_interface error\n");
          usb_close(com->dev);
          free(com);
          return NULL;
        }
        return com;
      }
    }
  }
  if (debug) 
    fprintf(stderr, "usbcom: device 0x%04x/0x%04x not found\n", vendor, product);
  free(com);
  return NULL;
}

void usbcom_close(usbcom_t com)
{
  usb_release_interface(com->dev, com->ino);
  usb_close(com->dev);
  free(com);
}

int usbcom_npipe(usbcom_t com)
{
  return com->npipe;
}

int usbcom_re_enumerate(usbcom_t com)
{
  /* XXX I don't know how to re-enumerate with libusb... */
  usb_find_devices();
  return 0;
}

int usbcom_send(usbcom_t com, int pipe, void *buf, int len)
{
  int    r;
  r = usb_bulk_write(com->dev, pipe, buf, len, 10000);
  return 0;
}

int usbcom_receive(usbcom_t com, int pipe, void *buf, int max_len)
{
  int    r;
  r = usb_bulk_read(com->dev, pipe, buf, max_len, 10000);
  if (debug) fprintf(stderr, "usbcom: max_len=%d r=%d\n", max_len, r);
  return r;
}

#endif /* LIBUSB */
