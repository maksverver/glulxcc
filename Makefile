include custom.mk

BINDIR="$(PREFIX)/bin/"
LIBEXECDIR="$(PREFIX)/lib/glulxcc/"
TARGETS=build/glulxas build/glulxld build/lcc build/cpp build/rcc

all: $(TARGETS)

install:
	mkdir -p $(BINDIR)
	mkdir -p $(LIBEXECDIR)
	install -s build/lcc $(BINDIR)/glulxcc
	install -s build/glulxas $(LIBEXECDIR)
	install -s build/glulxld $(LIBEXECDIR)
	install -s build/cpp $(LIBEXECDIR)
	install -s build/rcc $(LIBEXECDIR)


build/glulxas:
	make -C binutils glulxas

build/glulxld:
	make -C binutils glulxld

build/lcc:
	make -C lcc lcc

build/cpp:
	make -C lcc cpp

build/rcc:
	make -C lcc rcc

clean:
	make -C binutils clean
	make -C lcc clean

distclean: clean
	make -C binutils distclean
	make -C lcc clobber
	rm -f $(TARGETS)

.PHONY: all clean distclean install
