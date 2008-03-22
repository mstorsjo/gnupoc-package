CXXFLAGS += -Wall -pedantic -ggdb -Wno-long-long -Ideflate/deflate -Ideflate/inc
LDFLAGS += -lelf -lz

all: elf2e32 bindiff

elf2e32: elf2e32.o crc.o deflate/deflate.a
	$(CXX) -o $@ $+ $(LDFLAGS)

decompress: decompress.o deflate/deflate.a
	$(CXX) -o $@ $+

compress: compress.o deflate/deflate.a
	$(CXX) -o $@ $+

checksum: checksum.o
	$(CXX) -o $@ $+ -lz

deflate/deflate.a:
	$(MAKE) -C deflate deflate.a

bindiff: bindiff.o
	$(CXX) -o $@ $+

clean:
	$(MAKE) -C deflate clean
	rm -f elf2e32 bindiff *.o

