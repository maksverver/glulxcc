BUILDDIR=../build
export LCCDIR:=../build
CC=../build/lcc
CFLAGS=-I../include
B=$(BUILDDIR)/
O=.ulo

OBJS=	$Bcrtstart$O $Bctype$O $Bglk$O $Bintstr$O $Bprintf$O $Bstdlib1$O \
	$Bstring$O 

all: $Blibc.ulo

$Bcrtstart$O:	crtstart.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bctype$O:	ctype.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bglk.ula:	glk.m4;		m4 $< >$@ 
$Bglk$O:	$Bglk.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bintstr$O:	intstr.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bprintf$O:	printf.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bstdlib1$O:	stdlib1.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bstring$O:	string.c;	$(CC) $(CFLAGS) -c -o $@ $<

$Blibc.ulo: $(OBJS)
	cat $(OBJS) >$@

clean:
	rm -f $(OBJS) $Bglk.ula

distclean: clean
	rm -f ../build/libc.ulo

.PHONY: all clean distclean
