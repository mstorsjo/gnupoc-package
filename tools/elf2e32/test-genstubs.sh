#!/bin/sh

ARGS="_ZN10CMessenger11ShowMessageEv #<DLL>CreateStaticDll{000a0000}[0xA0000000].dll#<\\DLL>1b"

echo "ref.o $ARGS" | wine $EPOCROOT/epoc32/tools/genstubs.exe
echo "test.o $ARGS" | ./genstubs

compare() {
	CMD=$1
	ARGS=$2
	$CMD $ARGS ref.o > ref-out.txt
	$CMD $ARGS test.o | sed s/test.o/ref.o/ > test-out.txt
	diff -u ref-out.txt test-out.txt
}

compare arm-none-symbianelf-objdump -s
compare arm-none-symbianelf-readelf -sW
compare arm-none-symbianelf-objdump -r
compare arm-none-symbianelf-objdump -t

rm -f ref-out.txt test-out.txt ref.o test.o

