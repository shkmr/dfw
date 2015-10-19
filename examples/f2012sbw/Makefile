MCU     =    msp430x2012
TAPOPT  =    "MSP430 SBW VCC 3300"

CC      =    msp430-gcc
CFLAGS  =    -g -O -mmcu=$(MCU)

PROGS   =    stop_and_continue.elf \
             stack_bt.elf

all: $(PROGS)

stop_and_continue.elf : ../stop_and_continue.c
	$(CC) -o $@  $(CFLAGS) $<

stack_bt.elf          : ../stack_bt.c
	$(CC) -o $@  $(CFLAGS) $<

clean:
	-rm -f *~ *.elf *.o *.log

###
###  dmwt examples for MSP430F2012 with SpyBiWire JTAG
###
## standard 3.3V
upsc  : stop_and_continue.elf
	../../dmwt -c "MSP430 SBW VCC 3300" stop_and_continue.elf

## or use variables to make it look simple
## tak(10, 5, 1) will cause stack overflow, try smaller parameters.
upbt  : stack_bt.elf
	../../dmwt -c $(TAPOPT) stack_bt.elf

## backup:
## saves target's memory image into TITXT format.
## starting from address 0xf800 to 0xffff.
## titxt_compress will eliminate bulk of 0xff's.
bu  : 
	../../dmwt -c $(TAPOPT) -b foo.txt 0xf800
	../titxt_compress < foo.txt > backup.txt

## you can reload backup.txt 
re  : 
	../../dmwt -c $(TAPOPT) backup.txt

## you can also use Intel HEX file if you like,
uphex : stop_and_continue.ihex
	../../dmwt -c $(TAPOPT) stop_and_continue.ihex

###
### debug options
###
## verbose
upv     : stop_and_continue.elf
	../../dmwt -vc  $(TAPOPT) stop_and_continue.elf

## debug info
upvd    : stop_and_continue.elf
	../../dmwt -vdc $(TAPOPT) stop_and_continue.elf

###
###  gdbproxy
###
gp :
	msp430-gdbproxy --port=2000 uifdfw --c $(TAPOPT)

###
### debug option
###
### set debug flag of gdbproxy
gpg :
	msp430-gdbproxy --debug --port=2000 uifdfw --c $(TAPOPT)

### set debug flag of libdfw
gpd :
	msp430-gdbproxy --port=2000 uifdfw --c $(TAPOPT) --debug

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
