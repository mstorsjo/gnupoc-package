CXXFLAGS += -Wall -pedantic -ggdb -Wno-long-long
LDFLAGS += -lelf -lz

all: elf2e32 bindiff

elf2e32: elf2e32.o crc.o
	$(CXX) -o $@ $+ $(LDFLAGS)

bindiff: bindiff.o
	$(CXX) -o $@ $+

clean:
	rm -f elf2e32 bindiff *.o

