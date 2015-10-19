/* 
 *  USB com for MacOSX 
 *
 *                written by skimu@mac.com
 *
 *  Reference:
 *     http://developer.apple.com/documentation/DeviceDrivers/\
 *       Conceptual/USBBook/index.html#//apple_ref/doc/uid/TP40000973
 *     /Developer/Examples/IOKit/usb/
 */

#ifdef MacOSX

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOMessage.h>
#include <IOKit/usb/IOUSBLib.h>

#include "usbcom.h"

struct usbcom {
  IOUSBInterfaceInterface197 **intf;
  IOUSBDeviceInterface197 **dev;
  int  timeout;
  int  received_size;
};

static int debug = 0;
usbcom_set_debug_level(int x)
{
  debug = x;
}

/*
 *      Private functions
 */

static void set_number_to_dictionary(CFMutableDictionaryRef dic,
                              const void *key, SInt32 num)
{
  CFNumberRef n;
  
  
  n = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &num);

  CFDictionarySetValue(dic, key, n);

  CFRelease(n);
}

static IOUSBDeviceInterface197 **find_device(SInt32 vendor,
                                             SInt32 product)
{
  IOReturn r;
  SInt32 score;

  IOCFPlugInInterface        **plugin;
  IOUSBDeviceInterface197    **dev;

  CFMutableDictionaryRef dict;
  io_service_t service;
  io_iterator_t iter;

  dict = IOServiceMatching(kIOUSBDeviceClassName);
  set_number_to_dictionary(dict, CFSTR(kUSBVendorID),  vendor);
  set_number_to_dictionary(dict, CFSTR(kUSBProductID), product);

  service = IOServiceGetMatchingService(kIOMasterPortDefault, dict);

  if (!service)
    return NULL;

  r = IOCreatePlugInInterfaceForService(service,
                                        kIOUSBDeviceUserClientTypeID,
                                        kIOCFPlugInInterfaceID,
                                        &plugin,
                                        &score);

  if (r || !plugin)
    return NULL;

  r = (*plugin)->QueryInterface(plugin,
                             CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID197),
                             (LPVOID)&dev);

  IODestroyPlugInInterface(plugin);

  if (r || !dev)
    return NULL;

  return dev;
}
                                             

static IOReturn setup_device(IOUSBDeviceInterface197 **dev)
{
  IOReturn r;
  UInt8 nconf;

  IOUSBConfigurationDescriptorPtr config_desc;

  r = (*dev)->GetNumberOfConfigurations(dev, &nconf);
  if (r) goto cleanup;

  if (debug) 
    fprintf(stderr, "found %d configurations\n", nconf);

  /* XXX the last configuration hard-coded */
  r = (*dev)->GetConfigurationDescriptorPtr(dev, nconf - 1, &config_desc);
  if (r) goto cleanup;

  r = (*dev)->SetConfiguration(dev, config_desc->bConfigurationValue);
  if (r) goto cleanup;
    
  return 0;

 cleanup:
  return r;
}

static IOUSBInterfaceInterface197 **get_interface(IOUSBDeviceInterface197 **dev)
{
  IOReturn                   r;
  io_iterator_t              iter;
  io_service_t               service;
  IOUSBFindInterfaceRequest  req;
  IOCFPlugInInterface        **plugin;
  SInt32                     score;
  IOUSBInterfaceInterface197 **intf;

  req.bInterfaceClass     = kIOUSBFindInterfaceDontCare;
  req.bInterfaceSubClass  = kIOUSBFindInterfaceDontCare;
  req.bInterfaceProtocol  = kIOUSBFindInterfaceDontCare;
  req.bAlternateSetting   = kIOUSBFindInterfaceDontCare; 
  
  r = (*dev)->CreateInterfaceIterator(dev, &req, &iter); 
  if (r) return NULL;

  {
    service = IOIteratorNext(iter);

    r = IOCreatePlugInInterfaceForService(service,
                                          kIOUSBInterfaceUserClientTypeID, 
                                          kIOCFPlugInInterfaceID, 
                                          &plugin, &score);
    IOObjectRelease(service);
    if (r) goto cleanup;

    r = (*plugin)->QueryInterface(plugin,
                                  CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID),
                                  (LPVOID)&intf);
    IODestroyPlugInInterface(plugin);
    if (r) intf = NULL;
  }

 cleanup:

  IOObjectRelease(iter);

  return intf;
}

static void write_callback(void      *refcon,
                           IOReturn  result,
                           void      *arg0)
{
  if (debug) 
    fprintf(stderr, "write callback: result = %x\n", result);

  CFRunLoopStop(CFRunLoopGetCurrent());
}

static void read_callback(void      *refcon,
                          IOReturn  result,
                          void      *arg0)
{
  usbcom_t com = refcon;
  int i, size;

  if (debug) fprintf(stderr, "read callback\n");

  size = (long)arg0;

  if (result)
    fprintf(stderr, "read_callback: result = %x\n", result);

  com->received_size = size;

  if (debug)
    fprintf(stderr, "callback received = %d bytes\n", size);

  CFRunLoopStop(CFRunLoopGetCurrent());
}

static void print_pipe_properties(IOUSBInterfaceInterface197 **intf)
{
  IOReturn  r;
  UInt8 npipe;
  int i;

  (*intf)->GetNumEndpoints(intf, &npipe);
  
  fprintf(stderr, "found %d endpoints\n", npipe);

  for (i = 0; i < npipe + 1; i++) {
    UInt8   direction, number, transfer_type, interval;
    UInt16  max_packet_size;

    (*intf)->GetPipeProperties(intf, i, &direction, &number, 
                               &transfer_type, &max_packet_size, &interval);
    fprintf(stderr, "Pipe %d:\n", i);
    fprintf(stderr, "  Direction         = %d\n", direction);
    fprintf(stderr, "  Number            = %d\n", number);
    fprintf(stderr, "  Transfer Type     = %d\n", transfer_type);
    fprintf(stderr, "  Max Packet Size   = %d\n", max_packet_size);
    fprintf(stderr, "  Interval          = %d\n", interval);
  }
}

static void setup_notification(usbcom_t com)
{
  IOReturn r;
  CFRunLoopSourceRef    es;

  IOUSBInterfaceInterface197 **intf;

  intf = com->intf;
  
  com->received_size = 0;

  r = (*intf)->CreateInterfaceAsyncEventSource(intf, &es);

  CFRunLoopAddSource(CFRunLoopGetCurrent(), es, kCFRunLoopDefaultMode);
}

/*
 *     Public functions
 */

usbcom_t usbcom_open(int vendor_id, int product_id)
{
  IOReturn r;
  IOUSBDeviceInterface197 **dev;
  IOUSBInterfaceInterface197 **intf;
  usbcom_t com;
  UInt8 npipe;
  
  dev = find_device(vendor_id, product_id);
  if (!dev) return NULL;

  if (debug) fprintf(stderr, "device found\n");

  r = (*dev)->USBDeviceOpen(dev);
  if (r) return NULL;

  /* 
     r = (*dev)->ResetDevice(dev); sleep(1);
     if (r) return NULL;
  */

  if (debug) fprintf(stderr, "device opened\n");

  r = setup_device(dev);

  intf = get_interface(dev);

  if (!intf) goto cleanup;

  if (debug) fprintf(stderr, "interface found\n");

  if ((com = (usbcom_t)malloc(sizeof(struct usbcom))) == NULL)
    goto error;

  com->intf  = intf;
  com->dev   = dev;
  com->timeout = 500; /* ms */

  r = (*intf)->USBInterfaceOpen(intf);
  if (r) goto error;

  setup_notification(com);

  if (debug) 
    print_pipe_properties(intf);

  return com;

 error:

 cleanup:
  (*dev)->USBDeviceClose(dev);
  (*dev)->Release(dev);

  return NULL;
}

void usbcom_close(usbcom_t com)
{
  if (com == NULL) return;
  (*com->intf)->USBInterfaceClose(com->intf);
  (*com->intf)->Release(com->intf);
  (*com->dev)->USBDeviceClose(com->dev);
  (*com->dev)->Release(com->dev);
  free(com);
  com = NULL;
}

int usbcom_re_enumerate(usbcom_t com)
{
  (*com->dev)->USBDeviceReEnumerate(com->dev, 0);

  return 0;
}


int usbcom_npipe(usbcom_t com)
{
  UInt8 npipe;

  (*com->intf)->GetNumEndpoints(com->intf, &npipe);

  return (int)npipe;
}

int usbcom_send(usbcom_t com, int pipe, void *buf, int size)
{
  IOReturn r;

  
  if (debug) fprintf(stderr, "usbcom_send: size=%d\n", size);


  r = (*com->intf)->WritePipeAsync(com->intf, pipe, buf, size, 
                                   write_callback, com);

  if (r||debug) 
    warnx("usbcom_send: r = (0x%02x, 0x%03x, 0x%04x)",
          err_get_system(r), err_get_sub(r), err_get_code(r));


  CFRunLoopRun();

  return r;
}

int usbcom_receive(usbcom_t com, int pipe, void *buf, int size)
{
  IOReturn r;

  if (debug) fprintf(stderr, "receive: expecting %d bytes\n", size);

#if 1
  r = (*com->intf)->ReadPipeAsync(com->intf, pipe, buf, size, 
                                  read_callback, com);
#else
  r = (*com->intf)->ReadPipe(com->intf, pipe, buf, &size);
#endif
  
  if (r||debug) 
    warnx("usbcom_receive: r = (0x%02x, 0x%03x, 0x%04x)",
          err_get_system(r), err_get_sub(r), err_get_code(r));

  CFRunLoopRun();

  return com->received_size;
}

#endif /* MacOSX */

/* EOF */
