BUILDDIR=../build
export LCCDIR:=../build
CC=../build/lcc
CFLAGS=-I../include
B=$(BUILDDIR)/
O=.ulo
TARGETS=../include/glulx.h $Blibc.ulo

OBJS=	$Bassert$O $Batexit$O $Bcrtstart$O $Bctype$O $Bdivmod$O \
	$Bfgetc$O $Bfputc$O $Bfread$O $Bfwrite$O \
	$Bglulx$O $Bglk$O \
	$Bintstr$O $Bprintf$O $Bstdlib$O $Bstring$O

all: $(TARGETS)

$Bassert$O:	assert.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Batexit$O:	atexit.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bcrtstart$O:	crtstart.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bctype$O:	ctype.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bdivmod$O:	divmod.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bfgetc$O:	fgetc.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bfputc$O:	fputc.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bfread$O:	fread.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bfwrite$O:	fwrite.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bglk.ula:	glk.ula.m4;	m4 $< >$@ 
$Bglk$O:	$Bglk.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bglulx.ula:	glulx.ula.m4;	m4 $< >$@
$Bglulx$O:	$Bglulx.ula;	$(CC) $(CFLAGS) -c -o $@ $<
$Bintstr$O:	intstr.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bprintf$O:	printf.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bstdlib$O:	stdlib.c;	$(CC) $(CFLAGS) -c -o $@ $<
$Bstring$O:	string.ula;	$(CC) $(CFLAGS) -c -o $@ $<

$Blibc.ulo: $(OBJS)
	cat $(OBJS) >$@

../include/glulx.h: glulx.h.m4 glulx-operations.m4
	m4 $< >$@

clean:
	rm -f $(OBJS) $Bglk.ula $Bglulx.ula

distclean: clean
	rm -f $(TARGETS)

.PHONY: all clean distclean
