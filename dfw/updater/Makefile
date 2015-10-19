###
###
###
CC=msp430-gcc
LD=msp430-ld
MCU=msp430x1612
CFLAGS=-Os -mmcu=$(MCU) -DTARGET

OFILES=updater.o

ALL= updater_object_array.inc updater.lst

all : $(ALL)

updater_object_array.inc : updater.bin e
	./e updater.bin > updater_object_array.inc

updater.elf : $(OFILES)
	$(LD) -m $(MCU) -o updater.elf $(OFILES)	

e : e.c
	cc -o e e.c

clean:
	-rm -f $(ALL) *~ *.o *.s a.out e *.elf *.bin *.ihex 

.SUFFIXES : .elf .lst .bin .ihex

.elf.lst : 
	msp430-objdump -xDS $< > $@

.elf.bin : 
	msp430-objcopy -j .data -O binary $< $@

.elf.ihex :
	msp430-objcopy -O ihex $< $@
