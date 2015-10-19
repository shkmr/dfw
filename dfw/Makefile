##
##
##
all:    dfw.fetuif.ihex     \
        dfw.ez430u.ihex     \
        dfw.ez430u1p1.ihex  \
	ptfw.ez430u.ihex

clean:
	-@for i in conf/*; do (cd $$i; make clean); done
	-rm -f dfw.fetuif.elf dfw.ez430u.elf dfw.ez430u1p1.elf ptfw.ez430u.elf

very-clean:
	make clean;
	-rm -f dfw.fetuif.ihex dfw.ez430u.ihex dfw.ez430u1p1.ihex

dist : all
distclean: very-clean

dfw.fetuif.elf :
	(cd conf/fetuif; make)
	mv conf/fetuif/dfw dfw.fetuif.elf

dfw.ez430u.elf :
	(cd conf/ez430u; make)
	mv conf/ez430u/dfw dfw.ez430u.elf

ptfw.ez430u.elf :
	(cd conf/ez430u; make ptfw)
	mv conf/ez430u/ptfw ptfw.ez430u.elf

dfw.ez430u1p1.elf :
	(cd conf/ez430u1p1; make)
	mv conf/ez430u1p1/dfw dfw.ez430u1p1.elf

%.ihex : %.elf
	msp430-objcopy -O ihex $<  $@

### EOF
