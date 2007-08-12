#!/bin/sh

EPOCROOT=/home/martin/symbian-sdks/gnupoc-s60_30/
INPUT=$1
NAME=blahblahblah

ARGS="--definput=$INPUT --linkas=$NAME{000a0000}[f027392c].dll"

wine $EPOCROOT/epoc32/tools/elf2e32.exe --dso=e32-test.dso $ARGS 
mv e32-test.dso e32-ref.dso
./elf2e32 --dso=e32-test.dso $ARGS
#./bindiff -f e32-ref.dll -f e32-test.dll -i 0x14,0x18 -i 0x24,0x28
#rm e32-test.dll e32-ref.dll

