include custom.mk

BINDIR=$(PREFIX)/bin/
LIBDIR=$(PREFIX)/lib/glulxcc/
INCDIR=$(PREFIX)/include/glulxcc/
LIBEXECDIR=$(LIBDIR)
TARGETS=build/glulxas build/glulxld build/lcc build/cpp build/rcc build/libc.ulo

all: $(TARGETS)

install:
	mkdir -p $(BINDIR)
	mkdir -p $(LIBEXECDIR)
	install -s build/lcc $(BINDIR)/glulxcc
	install -s build/glulxas $(LIBEXECDIR)
	install -s build/glulxld $(LIBEXECDIR)
	install -s build/cpp $(LIBEXECDIR)
	install -s build/rcc $(LIBEXECDIR)
	cp build/libc.ulo $(LIBDIR)
	mkdir -p $(INCDIR)
	for file in `ls include`; do cp include/"$$file" $(INCDIR); done

uninstall:
	rm -f $(BINDIR)/glulxcc
	rm -f $(LIBEXECDIR)/{glulxas,glulxld,cpp,rcc}
	rm -f $(LIBDIR)/libc.ulo
	rmdir $(LIBEXECDIR)
	for file in `ls include`; do rm -f $(INCDIR)/$$file; done
	rmdir $(INCDIR)

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

build/libc.ulo:
	make -C lib all

clean:
	make -C binutils clean
	make -C lcc clean

distclean: clean
	make -C binutils distclean
	make -C lcc clobber
	rm -f $(TARGETS)

.PHONY: all clean distclean install $(TARGETS)
