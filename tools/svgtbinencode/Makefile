CFLAGS = -Wall -pedantic -Iexpat-2.0.1/lib
CXXFLAGS = $(CFLAGS)
LDFLAGS =

all: decompile encode

decompile: decompile.o
	$(CXX) -o $@ $+ $(LDFLAGS)

encode: encode.o ConvertUTF.o expat-2.0.1/.libs/libexpat.a pathcommands.o pathcommands-tokenizer.o write.o style-tokenizer.o
	$(CXX) -o $@ $+ $(LDFLAGS)

pathcommands.cpp: pathcommands.y
	bison --verbose --debug -d -o $@ -p pathcommands $<
	if [ ! -f pathcommands.hpp ]; then ln -s pathcommands.cpp.h pathcommands.hpp; fi

pathcommands-tokenizer.c: pathcommands-tokenizer.l
	flex -Ppathcommands -o$@ $<

pathcommands: pathcommands.o pathcommands-tokenizer.o
	$(CXX) -o $@ $+

style-tokenizer.c: style-tokenizer.l
	flex -Pstyle -o$@ $<

expat-2.0.1/.libs/libexpat.a:
	cd expat-2.0.1 && ./configure --enable-shared=no && make buildlib

clean:
	rm -f decompile encode *.o

clean-all: clean
	rm -f pathcommands.cpp pathcommands.hpp pathcommands-tokenizer.c style-tokenizer.c


