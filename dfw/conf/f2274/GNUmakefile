#
#
#
MCU     =  msp430x2274
#MCU    =  msp430x2618

CC      =  msp430-gcc
DEFS    =  
CFLAGS  =  -g -O -Wall -mmcu=$(MCU) $(DEFS)

PROG    =  dfw writemini ebfw.elf

###
###
###
OBJS    =  dfw.o cmds.o updater.o       \
	   target.o target_msp430.o     \
	            target_cpuX.o       \
	            target_cpuXv2.o     \
	            target_cc8051.o     \
	   tap.o tap_jtag.o tap_sbw.o   \
	   uif_f2274.o                  \
	   datalink.o

dfw : $(OBJS)
	$(CC) $(CFLAGS) -o dfw $(OBJS)

UIF_H       =  uif_macros.h
dfw.o       : dfw.c      $(UIF_H)
cmds.o      : ../../cmds.c     $(UIF_H)
updater.o   : updater.c
datalink.o  : datalink.c       $(UIF_H)
eeprom.o    : ../../eeprom.c   $(UIF_H)
uif_f2274.o : uif_f2274.c      $(UIF_H)
tap.o       : ../../tap.c      $(UIF_H)
tap_jtag.o  : ../../tap_jtag.c $(UIF_H)
tap_sbw.o   : ../../tap_sbw.c  $(UIF_H)
target.o    : ../../target.c            ../../target.h 
target_msp430.o : ../../target_msp430.c ../../target.h ../../jtag89.h
target_cpuX.o   : ../../target_cpuX.c   ../../target.h ../../jtag89.h
target_cpuXv2.o : ../../target_cpuXv2.c ../../target.h ../../jtag91.h
target_cc8051.o : ../../target_cc8051.c ../../target_cc8051.h

###
###
###
EBOBJS  =  ebfw.o                       \
	   uif_f2274.o                  \
	   datalink.o

ebfw.elf : $(EBOBJS)
	$(CC) $(CFLAGS) -o ebfw.elf $(EBOBJS)

ebfw.ihex :

%.ihex : %.elf
	msp430-objcopy -O ihex $<  $@

###
###
###
#FET   = fetuif
FET    = ez430u
#FET   = ez430u1p1

MINI   = ../$(FET)/minidfw
#MINI  = ../$(FET)/dfw

WMOBJS  =  writemini.o                  \
	   target_msp430.o              \
	   tap.o tap_jtag.o tap_sbw.o   \
	   uif_f2274.o                  

writemini : $(WMOBJS)
	$(CC) $(CFLAGS) -o writemini $(WMOBJS)
writemini.o : minidfw_text.inc

minidfw_text.inc : $(MINI) ../../updater/e
	msp430-objdump -h $< | awk -f getLMA.awk > minidfw_lma.inc
	msp430-objcopy -j .text    -O binary $< minidfw_text.bin
	msp430-objcopy -j .data    -O binary $< minidfw_data.bin
	msp430-objcopy -j .vectors -O binary $< minidfw_vect.bin
	../../updater/e minidfw_text.bin > minidfw_text.inc
	../../updater/e minidfw_data.bin > minidfw_data.inc
	../../updater/e minidfw_vect.bin > minidfw_vect.inc

../$(FET)/minidfw :
	(cd ../$(FET); make minidfw)

../$(FET)/dfw :
	(cd ../$(FET); make dfw)

../../updater/e :
	(cd ../../updater; make e)

###
###
###
clean:
	(cd ../../updater; make clean)
	-rm -f $(PROG) *~ *.o *.lst *.inc *.bin

.c.s :
	$(CC) -S $(CFLAGS) $<

%.o : %.c
	$(CC) $(CFLAGS) -I../.. -c $< -o $@

%.o : ../../%.c
	$(CC) $(CFLAGS) -I. -c $< -o $@

### EOF
