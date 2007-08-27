#!/bin/sh

INPUT=$1

ARGS="--definput=$INPUT --linkas=libname{000a0000}[f027392c].dll"

wine $EPOCROOT/epoc32/tools/elf2e32.exe --dso=e32-test.dso $ARGS 
mv e32-test.dso e32-ref.dso
./elf2e32 --dso=e32-test.dso $ARGS
#./bindiff e32-ref.dll e32-test.dll -i 0x14,0x18 -i 0x24,0x28

