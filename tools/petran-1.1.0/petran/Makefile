#
# Makefile
#

# GCC_VERSION=-2.95
CFLAGS+=-Wall -gstabs+ -Wno-deprecated
CFLAGS+=-I../inc
LFLAGS=
DEPS=\
	../inc/pe_defs.h \
	../inc/e32def.h \
	../inc/e32image.h \
	../inc/e32rom.h \
	../inc/e32std.h \
	../inc/e32uid.h \
	../inc/pe_file.h \
	../inc/h_utl.h \
	../inc/h_ver.h

OBJS=\
	petran.o \
	../e32uid/e32uid.o \
	../host/h_utl.o \
	../host/h_file.o \
	../host/h_mem.o \
	../pefile/pe_img.o \
	../pefile/pe_imp.o \
	../pefile/pe_dump.o \
	../pefile/pe_utl.o \
	../pefile/pe_io.o \
	../pefile/pe_tran.o \
	../pefile/pe_reloc.o \
	../pefile/pe_file.o

BIN=petran

all: $(BIN)

$(BIN): $(OBJS) Makefile
	g++$(GCC_VERSION) $(LFLAGS) $(OBJS) -o $@ $(LIBS)

%.o: %.cpp $(DEPS)
	g++$(GCC_VERSION) $(CFLAGS) -c $< -o $@

%.o: %.c
	gcc$(GCC_VERSION) $(CFLAGS) -c $< -o $@

install: $(BIN)
	install -m 755 $(BIN) $(DESTDIR)/usr/bin

uninstall:
	rm -f $(DESTDIR)/usr/bin/$(BIN)

clean:
	rm -f $(OBJS) $(BIN) core ../*/*~ ../*/*.o
