BUILDDIR=../build
export LCCDIR:=../build
CC=../build/lcc
CFLAGS=-I../include
B=$(BUILDDIR)/
O=.ulo

OBJS=	$Bassert$O $Bcrtstart$O $Bctype$O $Bglulx$O $Bglk$O $Bintstr$O \
	$Bprintf$O $Bstdlib$O $Bstring$O

all: $Blibc.ulo

$Bassert$O:	assert.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bcrtstart$O:	crtstart.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bctype$O:	ctype.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bglulx$O:	glulx.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bglk.ula:	glk.m4;		m4 $< >$@ 
$Bglk$O:	$Bglk.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bintstr$O:	intstr.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bprintf$O:	printf.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bstdlib$O:	stdlib.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bstring$O:	string.c;	$(CC) $(CFLAGS) -c -o $@ $<

$Blibc.ulo: $(OBJS)
	cat $(OBJS) >$@

clean:
	rm -f $(OBJS) $Bglk.ula

distclean: clean
	rm -f ../build/libc.ulo

.PHONY: all clean distclean
