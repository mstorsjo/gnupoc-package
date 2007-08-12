#
# Makefile
#

# GCC_VERSION=-2.95
CFLAGS+=-Wall -gstabs+
CFLAGS+=-I../inc
LFLAGS=
DEPS=\
	../inc/e32uid.h \
	../inc/e32std.h

OBJS=\
	e32uid.o \
	uidcrc.o

BIN=uidcrc

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
	rm -f $(OBJS) $(BIN) core
