###
## GCC
##  gcc-3.2.3 produces smaller code (especially for updater, 958 bytes),
##
##  gcc-4.4.3 produces larger code (updater becomes 1402 bytes)
##            and -O -O2 does not work, -Os works for eZ430U
##            As for FET430UIF, even -Os have problem on i2c routine
##            TUSB3410 does not start up well and need to put tusb3410.bin
##            manually.  I haven't investigated further.
##
CC      =  msp430-gcc
#CC     =  msp430-gcc-3.2.3
#CC     =  msp430-gcc-4.4.3
##
CFLAGS  =  -g -O -Wall -mmcu=msp430x1612
#CFLAGS =  -g -O -Wall -mmcu=msp430x1612 -DTUSB3410_USB_HEADER_ONLY

PROG    =  dfw minidfw
OBJS    =  dfw.o cmds.o updater.o       \
	   target.o target_msp430.o     \
	            target_cpuX.o       \
	            target_cpuXv2.o     \
	            target_cc8051.o     \
	   tap.o tap_jtag.o tap_sbw.o   \
	   uif_fet.o                    \
	   datalink.o eeprom.o

dfw : $(OBJS)
	$(CC) $(CFLAGS) -o dfw $(OBJS)

UIF_H       =  uif_macros.h
dfw.o       : ../../dfw.c      $(UIF_H)
cmds.o      : ../../cmds.c     $(UIF_H)
updater.o   : ../../updater.c  ../../updater/updater_object_array.inc
datalink.o  : ../../datalink.c $(UIF_H)
eeprom.o    : ../../eeprom.c   $(UIF_H)
uif_fet.o   : uif_fet.c        $(UIF_H)
tap.o       : ../../tap.c      $(UIF_H)
tap_jtag.o  : ../../tap_jtag.c $(UIF_H)
tap_sbw.o   : ../../tap_sbw.c  $(UIF_H)
target.o    : ../../target.c            ../../target.h 
target_msp430.o : ../../target_msp430.c ../../target.h ../../jtag89.h
target_cpuX.o   : ../../target_cpuX.c   ../../target.h ../../jtag89.h
target_cpuXv2.o : ../../target_cpuXv2.c ../../target.h ../../jtag91.h
target_cc8051.o : ../../target_cc8051.c
minidfw.o   : ../../minidfw.c

MINIOBJS = minidfw.o updater.o datalink.o uif_fet.o eeprom.o
minidfw : $(MINIOBJS)
	$(CC) $(CFLAGS) -o minidfw $(MINIOBJS)

../../updater/updater_object_array.inc : ../../updater/updater.c
	(cd ../../updater; make CC=$(CC) updater_object_array.inc)

clean:
	(cd ../../updater; make clean)
	-rm -f $(PROG) *~ *.o *.lst

.c.s :
	$(CC) -S $(CFLAGS) $<

%.o : %.c
	$(CC) $(CFLAGS) -I../.. -c $< -o $@

%.o : ../../%.c
	$(CC) $(CFLAGS) -I. -c $< -o $@

### EOF
