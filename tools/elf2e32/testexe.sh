#!/bin/sh

INPUT=$1

ARGS="--uncompressed --sid=0x00000000 --uid1=0x1000007a --uid2=0x00000000 --uid3=0x00000000 --vid=0x00000000 --capability=none --fpu=softvfp --targettype=EXE --linkas=test{000a0000}.exe --libpath=${EPOCROOT}/epoc32/release/armv5/lib/ --elfinput=$INPUT"

wine $EPOCROOT/epoc32/tools/elf2e32.exe $ARGS --output=e32-ref.exe 
./elf2e32 $ARGS --output=e32-test.exe
./bindiff -f e32-ref.exe -f e32-test.exe -i 0x14,0x18 -i 0x24,0x28
rm e32-test.exe e32-ref.exe

