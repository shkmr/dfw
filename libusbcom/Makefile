###
### generic unix
###
#CFLAGS = -DLIBUSB
#OFILES = usbcom_libusb.o

###  
###   MacOSX
### 
CFLAGS = -DMacOSX
OFILES = usbcom_mac.o

all : libusbcom.a

libusbcom.a : $(OFILES)
	ar cru libusbcom.a $(OFILES)
	ranlib libusbcom.a

usbcom_mac.o    : usbcom_mac.c    usbcom.h
usbcom_libusb.o : usbcom_libusb.c usbcom.h

clean:
	-rm -f libusbcom.a *.o *~

