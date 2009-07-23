CFLAGS=-Wall -g -O2
ASOBJS=parser.tab.o lex.yy.o operations.o glulxas.o
LDOBJS=glulxld.o

all: glulxas glulxld

parser.tab.c: parser.y
	yacc -d parser.y

lex.yy.c: scanner.l
	lex scanner.l

glulxas: $(ASOBJS)
	$(CC) -o glulxas $(ASOBJS)

glulxld: $(LDOBJS)
	$(CC) -o glulxld $(LDOBJS)

clean:
	rm -f $(ASOBJS) $(LDOBJS) parser.tab.c lex.yy.c

distclean: clean
	rm -f glulxas glulxld

.PHONY: all clean distclean
