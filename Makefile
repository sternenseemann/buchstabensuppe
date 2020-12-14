PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib
INCDIR ?= $(PREFIX)/include

.PHONY: all clean install check
all:
	redo bs-renderflipdot.exe demo.exe libbuchstabensuppe.a

clean:
	rm -f *.o *.a *.exe third_party/stb_truetype.o

check:
	redo test.exe
	./test.exe

install:
	install -Dm755 demo.exe $(BINDIR)/buchstabensuppe-demo
	install -Dm644 include/buchstabensuppe.h -t $(INCDIR)
	install -Dm644 include/buchstabensuppe/bitmap.h -t $(INCDIR)/buchstabensuppe
	install -Dm644 third_party/stb/stb_truetype.h -t $(INCDIR)
	install -Dm644 libbuchstabensuppe.a -t $(LIBDIR)
