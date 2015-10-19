#
# Makefile	INTELHex to Flatfile (binary) converter
#

CC =		gcc
CFLAGS =	-c -Wall

OBJS=	main.o ihex.o

ihex2flat:	$(OBJS)
	$(CC) $(OBJS) -o ihex2flat

clean:
	rm -f ihex2flat *.o *~

main.o:	main.c
	$(CC) $(CFLAGS) main.c -o main.o

ihex.o:	ihex.c
	$(CC) $(CFLAGS) ihex.c -o ihex.o
