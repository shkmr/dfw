MCU     =    msp430x5529
TAPOPT  =    "CPUXV2 SBW VCC 3300"

CC      =    msp430-gcc
CFLAGS  =    -ggdb -O -mmcu=$(MCU)  -mcode-64k -mdata-64k

PROGS   =    stop_and_continue.elf \
             stack_bt.elf

all: $(PROGS)

stop_and_continue.elf : stop_and_continue.c
	$(CC) -o $@  $(CFLAGS) $<

stack_bt.elf          : stack_bt.c
	$(CC) -o $@  $(CFLAGS) $<

clean:
	-rm -f *~ *.elf *.o *.log

upv     : stop_and_continue.elf
	../../dmwt -vc  $(TAPOPT) stop_and_continue.elf

###
###
###
distclean:
	-rm -f *~ *.elf *.o *.log *.txt *.ihex

.SUFFIXES : .elf .lst .ihex

.elf.lst : 
	msp430-objdump -xDS $< > $@

.elf.ihex :
	msp430-objcopy -O ihex $< $@
