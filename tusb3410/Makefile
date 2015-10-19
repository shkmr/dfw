#
# Makefile	tusb3410 firmware
#
VID='\#x0451'
PID='\#xbeef'

CC =		/usr/local/bin/sdcc
LD =		/usr/local/bin/sdcc
LFLAGS =	--model-small --idata-loc 0xc0 --xram-loc 0xf800 --xram-size 2048 --code-size 16384
#		                            ^ info for weasels only: the stack grows upwards ...
CFLAGS =	-c

OBJS=	main.rel usb.rel ser.rel

all : firm.flat tusb3410_autoexec_firmware.inc tusb3410_descriptor_header.inc 

tusb3410_autoexec_firmware.inc : firm.flat
	gosh gen-descriptor-header.scm $(VID) $(PID) firm.flat

tusb3410_descriptor_header.inc : gen-descriptor-header.scm
	gosh gen-descriptor-header.scm $(VID) $(PID)

firm.flat : main.ihx ihex/ihex2flat
	ihex/ihex2flat main.ihx firm.flat

main.ihx : $(OBJS) Makefile ihex/ihex2flat
	$(LD) $(LFLAGS) $(OBJS)

ihex/ihex2flat:
	(cd ihex; make)

clean:	
	rm -f firm.flat *.asm *.ihx *.lnk *.map *.mem *.lst *.rel *.rst *.sym
	rm -f *~	
	(cd ihex; make clean)

main.rel: main.c Makefile
	$(CC) $(CFLAGS) main.c

usb.rel: usb.c Makefile tusb3410_descriptor_header.inc
	$(CC) $(CFLAGS) usb.c

ser.rel: ser.c Makefile
	$(CC) $(CFLAGS) ser.c


