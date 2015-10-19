###
###    BFD library location
###
BFDINC    =  -I/usr/local/mspgcc3/include
BFDLIB    =  -L/usr/local/mspgcc3/lib/x86_64 -lbfd -liberty -lz
##
#BINUTILS_SRC=/home/skimu/distfiles/binutils-2.19
#BINUTILS_LIB=/home/skimu/distfiles/build-binutils
#BFDINC=-I$(BINUTILS_SRC)/include -I$(BINUTILS_LIB)/bfd 
#BFDLIB=-L$(BINUTILS_LIB)/bfd -L$(BINUTILS_LIB)/libiberty  \
#       -lbfd -liberty -lz
##
#BINUTILS_SRC=/Volumes/data/src/binutils-2.19
#BINUTILS_LIB=/Volumes/data/src/binutils-2.19
#BFDINC=-I$(BINUTILS_SRC)/include -I$(BINUTILS_LIB)/bfd 
#BFDLIB=-L$(BINUTILS_LIB)/bfd -L$(BINUTILS_LIB)/libiberty  \
#       -L$(BINUTILS_LIB)/intl -lbfd -liberty -lintl -lz

###
###    USB library for boot3410.c (not required for standard installation)
###
COMINC     =  -I./libusbcom
COMLIB     =  -L./libusbcom -lusbcom -framework CoreFoundation -framework IOKit
## need to change libusbcom/Makefile as well
#COMLIB    =  -L./libusbcom -lusbcom -lusb

###
### HOST specific definitions (if any)
###   
CFLAGS    = -O
LDFLAGS   = 

###
###
###
all        : dmwt titxt2ihex titxt_compress bufet
all-debug  : dfwsh boot3410

###
###
###
DMWTOBJS= dmwt.o dfw/libdfw.o segment.o elfbfd.o titxt.o hex.o

dmwt : $(DMWTOBJS)
	cc -o dmwt $(DMWTOBJS) $(BFDLIB) $(LDFLAGS)

dmwt.o       : segment.h dfw/libdfw.h hex.h
segment.o    : segment.h
dfw/libdfw.o : dfw/libdfw.c
elfbfd.o     : elfbfd.c segment.h
	cc $(CFLAGS) $(BFDINC) -c elfbfd.c -o elfbfd.o

## utilities
titxt2ihex : titxt.c hex.o
	cc -o titxt2ihex -DTITXT_STANDALONE titxt.c hex.o

titxt_compress : titxt_compress.o titxt.o
	cc -o titxt_compress titxt_compress.o titxt.o

## debug tools
# In some system, dfwsh needs -lcurses as well.
dfwsh      : dfwsh.c
	cc -o dfwsh dfwsh.c -lreadline

bufet      : bufet.c segment.o titxt.o dfw/libdfw.o bufetee/bufetee_object_array.inc
	cc $(CFLAGS) -o bufet bufet.c segment.o titxt.o dfw/libdfw.o

bufetee/bufetee_object_array.inc :
	(cd bufetee; make)

boot3410   : boot3410.c libusbcom/usbcom.h libusbcom/libusbcom.a
	cc -o boot3410 $(COMINC) -DBOOT3410_STANDALONE   \
                      boot3410.c $(COMLIB) $(LDFLAGS)

libusbcom/libusbcom.a : libusbcom/usbcom.h
	(cd libusbcom; make)

###
###
###
clean :
	-rm -f $(TARGETS) *.o dfw/libdfw.o *.~*~ *~
	-@for i in libusbcom bufetee;  do                   \
	  (cd $$i; echo cleaning $$i; make clean);  \
	done

distclean: clean
	-@(cd dfw; make clean)
	-@(cd tusb3410; make clean)
	-@(cd examples; make clean)
	-@(cd gdbproxy; make distclean)

.c.o :
	cc $(CFLAGS) -c $< -o $@

### EOF
