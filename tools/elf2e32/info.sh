#!/bin/sh

EPOCROOT=/home/martin/symbian-sdks/gnupoc-s60_30/
INPUT=$1
ARGS="--uncompressed --sid=0x00000000 --uid1=0x1000007a --uid2=0x00000000 --uid3=0x00000000 --vid=0x00000000 --capability=DRM+TCB+AllFiles --fpu=softvfp --targettype=EXE --linkas=test{000a0000}.exe --libpath=${EPOCROOT}/epoc32/release/armv5/lib/ --elfinput=$INPUT"

wine $EPOCROOT/epoc32/tools/elf2e32.exe $ARGS --output=e32-ref.exe 
wine $EPOCROOT/epoc32/tools/elf2e32.exe --e32input=e32-ref.exe | less
rm e32-ref.exe

