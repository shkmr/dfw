MCU     =    msp430x169
TAPOPT  =    "MSP430 JTAG VCC 3300 SlowFlash"

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
###  dmwt examples for MSP430F169
###
## standard 3.3V
upsc  : stop_and_continue.elf
	../../dmwt -c "MSP430 JTAG VCC 3300 SlowFlash" stop_and_continue.elf

upbt  : stack_bt.elf
	../../dmwt -c "MSP430 JTAG VCC 3300 SlowFlash" stack_bt.elf


## Options given above was default value.
## you don't have to put -c for f16x
upf : stop_and_continue.elf
	../../dmwt stop_and_continue.elf

## VCC 3.6V:
uph : stop_and_continue.elf
	../../dmwt -c "MSP430 JTAG VCC 3600" stop_and_continue.elf

## backup:
## saves target's memory image into TITXT format.
## starting from address 0x1100 to 0xffff.
## titxt_compress will eliminate bulk of 0xff's.
bu  : 
	../../dmwt -c $(TAPOPT) -b foo.txt 0x1100
	../titxt_compress < foo.txt > backup.txt

## you can reload backup.txt 
re  : 
	../../dmwt -c $(TAPOPT) backup.txt

## Note: backing up your target FET
##
##   F1612's flash starts from 0x2500.
##   To restore, you need to give -f (firmware update)
##   option to dmwt command like,
##   dmwt -f backup.txt


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

## more debug info
upvdd   : stop_and_continue.elf
	../../dmwt -vdc "JTAG MSP430 DEBUG" stop_and_continue.elf

## even more debug info
upvdmax : stop_and_continue.elf
	../../dmwt -vddddc "JTAG MSP430 DEBUG DEBUG" stop_and_continue.elf

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
