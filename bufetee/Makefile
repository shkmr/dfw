##
##
##
CC      =  msp430-gcc
LD      =  msp430-ld
MCU     =  msp430x1612
CFLAGS  =  -O -mmcu=$(MCU)
ED      = ../dfw/updater
E       = $(ED)/e

OFILES  =  bufetee.o

all : bufetee_object_array.inc

bufetee.elf : $(OFILES)
	$(LD) -m $(MCU) -o bufetee.elf $(OFILES)	

bufetee_object_array.inc : bufetee.bin $(E)
	$(E) bufetee.bin > bufetee_object_array.inc

$(E) :
	(cd $(ED); make e)

.SUFFIXES : .elf .lst .bin .ihex

.elf.lst : 
	msp430-objdump -xDS $< > $@

.elf.bin : 
	msp430-objcopy -j .data -O binary $< $@

.elf.ihex :
	msp430-objcopy -O ihex $< $@

clean:
	rm -f *~ *.o *.elf *.lst *.bin *.inc


