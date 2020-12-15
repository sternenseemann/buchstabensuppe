PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib
INCDIR ?= $(PREFIX)/include
MANDIR ?= $(PREFIX)/share/man

.PHONY: all clean install check
all:
	redo bs-renderflipdot.exe libbuchstabensuppe.a

clean:
	rm -f *.o *.a *.exe third_party/stb_truetype.o

check:
	redo test.exe
	./test.exe

install:
	install -Dm755 bs-renderflipdot.exe $(BINDIR)/bs-renderflipdot
	install -Dm644 include/buchstabensuppe.h -t $(INCDIR)
	install -Dm644 include/buchstabensuppe/bitmap.h -t $(INCDIR)/buchstabensuppe
	install -Dm644 third_party/stb/stb_truetype.h -t $(INCDIR)
	install -Dm644 libbuchstabensuppe.a -t $(LIBDIR)
	install -Dm644 doc/man/bs-renderflipdot.1 -t $(MANDIR)/man1
