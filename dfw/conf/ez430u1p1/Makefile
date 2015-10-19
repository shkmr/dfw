##
##
##
MCU     =  msp430x1612
CC      =  msp430-gcc

DEFS    =  -DEZ430U1P1
PROG    =  dfw
OBJS    =  dfw.o cmds.o updater.o       \
	   target.o target_msp430.o     \
	            target_cpuX.o       \
	            target_cpuXv2.o     \
	            target_cc8051.o     \
	   tap.o tap_jtag.o tap_sbw.o   \
	   uif_ez.o                     \
	   datalink.o eeprom.o

CFLAGS  =  -g -O -Wall -mmcu=$(MCU) $(DEFS)

dfw : $(OBJS) ../../updater/updater_object_array.inc
	$(CC) $(CFLAGS) -o dfw $(OBJS)

UIF_H       = ../ez430u/uif_macros.h
dfw.o       : ../../dfw.c      $(UIF_H)
cmds.o      : ../../cmds.c     $(UIF_H)
updater.o   : ../../updater.c  ../../updater/updater_object_array.inc
datalink.o  : ../../datalink.c $(UIF_H)
eeprom.o    : ../../eeprom.c   $(UIF_H)
tap.o       : ../../tap.c      $(UIF_H)
tap_jtag.o  : ../../tap_jtag.c $(UIF_H)
tap_sbw.o   : ../../tap_sbw.c  $(UIF_H)
target.o        : ../../target.c        ../../target.h
target_msp430.o : ../../target_msp430.c ../../target.h ../../jtag89.h
target_cpuX.o   : ../../target_cpuX.c   ../../target.h ../../jtag89.h
target_cpuXv2.o : ../../target_cpuXv2.c ../../target.h ../../jtag91.h
target_cc8051.o : ../../target_cc8051.c
uif_ez.o    : ../ez430u/uif_ez.c $(UIF_H)

../../updater/updater_object_array.inc : ../../updater/updater.c
	(cd ../../updater; make CC=$(CC) updater_object_array.inc)

clean:
	(cd ../../updater; make clean)
	-rm -f $(PROG) *~ *.o *.lst

.c.s :
	$(CC) -S $(CFLAGS) $<

%.o : ../ez430u/%.c
	$(CC) -c $(CFLAGS) -I../.. $< -o $@

%.o : ../../%.c
	$(CC) -c $(CFLAGS) -I../ez430u $< -o $@

### EOF
