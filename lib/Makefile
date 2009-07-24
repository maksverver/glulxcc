BUILDDIR=../build
export LCCDIR:=../build
CC=../build/lcc
CFLAGS=-I../include
B=$(BUILDDIR)/
O=.ulo
OBJS=$Bcrtstart$O $Bglk$O $Bprintf$O $Bstring$O

all: $Blibc.ulo

$Bglk.ula: glk.m4;		m4 $< >$@ 
$Bglk$O:	$Bglk.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bcrtstart$O:	crtstart.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bprintf$O:	printf.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bstring$O:	string.c;	$(CC) $(CFLAGS) -c -o $@ $<

$Blibc.ulo: $(OBJS)
	cat $(OBJS) >$@

clean:
	rm -f $(OBJS) $Bglk.ula

distclean: clean
	rm -f ../build/libc.ulo

.PHONY: all clean distclean
