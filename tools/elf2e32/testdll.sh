#!/bin/sh

INPUT=$1

ARGS="--sid=0x00000000 --uid1=0x10000079 --uid2=0x1000008d --uid3=0x00000000 --vid=0x00000000 --capability=none --fpu=softvfp --targettype=DLL --elfinput=$INPUT --linkas=libname{000a0000}[00000000].dll --libpath=$EPOCROOT/epoc32/release/armv5/lib/"

wine $EPOCROOT/epoc32/tools/elf2e32.exe --output=e32-ref.dll --defoutput=e32-ref.def --dso=e32-ref.dso --uncompressed $ARGS 
wine $EPOCROOT/epoc32/tools/elf2e32.exe --e32input=e32-ref.dll > e32info.txt
./elf2e32 --output=e32-test.dll --defoutput=e32-test.def --dso=e32-test.dso $ARGS
./bindiff -f e32-ref.dll -f e32-test.dll -i 0x14,0x18 -i 0x24,0x28
diff -u e32-ref.def e32-test.def
rm e32-test.dll e32-ref.dll

