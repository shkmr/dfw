SUBDIRS  = f169 f2012sbw f2274jtag f2274sbw f2618jtag f5529sbw

all:

clean:
	-rm -f *~
	-@for i in $(SUBDIRS); do              \
	     echo $$i;                         \
             (cd $$i; make clean);             \
	 done
