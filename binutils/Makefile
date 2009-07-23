#CC=gcc
CFLAGS=-Wall -g -O2 -I.
LDFLAGS=-g
BUILDDIR=../build

B=$(BUILDDIR)/
O=.o

ASOBJS=$Bparser.tab$O $Bscanner.yy$O $Boperations$O $Bglulxas$O
LDOBJS=$Bglulxld$O
OUTPUT=$Bglulxas $Bglulxld

all: $(OUTPUT)

glulxas: $Bglulxas
	@true

glulxld: $Bglulxld
	@true

$Bparser.tab.c: parser.y
	yacc -o "$@" -d "$<"

$Bscanner.yy.c: scanner.l
	lex -o "$@" "$<"

$Bglulxas$O: glulxas.c;		$(CC) $(CFLAGS) -c -o "$@" "$<"
$Bglulxld$O: glulxld.c;		$(CC) $(CFLAGS) -c -o "$@" "$<"
$Boperations$O: operations.c;	$(CC) $(CFLAGS) -c -o "$@" "$<"

$Bglulxas: $(ASOBJS)
	$(CC) $(LDFLAGS) -o $Bglulxas $(ASOBJS)

$Bglulxld: $(LDOBJS)
	$(CC) $(LDFLAGS) -o $Bglulxld $(LDOBJS)

clean:
	rm -f $(ASOBJS) $(LDOBJS) $Bparser.tab.c $Bparser.tab.h $Bscanner.yy.c

distclean: clean
	rm -f $(OUTPUT)

.PHONY: all clean distclean
