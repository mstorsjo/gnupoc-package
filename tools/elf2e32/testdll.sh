#!/bin/sh

EPOCROOT=/home/martin/symbian-sdks/gnupoc-s60_30/
INPUT=$1

#elf2e32 --definput="../../../symbian-sdks/gnupoc-s60_30/epoc32/build/home/martin/code/blaster/group/puttyengine/gcce/puttyengine{000a0000}.prep.def" --dso=../../../symbian-sdks/gnupoc-s60_30/epoc32/release/armv5\\lib\\puttyengine{000a0000}.dso --linkas=puttyengine{000a0000}[f027392c].dll


ARGS="--sid=0xf027392c --uid1=0x10000079 --uid2=0x1000008d --uid3=0xf027392c --vid=0x00000000 --capability=LocalServices --fpu=softvfp --targettype=DLL --elfinput=$INPUT --linkas=libname{000a0000}[f027392c].dll --libpath=../../symbian-sdks/gnupoc-s60_30/epoc32/release/armv5/lib/"

wine $EPOCROOT/epoc32/tools/elf2e32.exe --output=e32-ref.dll --defoutput=e32-ref.def --dso=e32-ref.dso --uncompressed $ARGS 
wine $EPOCROOT/epoc32/tools/elf2e32.exe --e32input=e32-ref.dll > e32info.txt
./elf2e32 --output=e32-test.dll --defoutput=e32-test.def --dso=e32-test.dso $ARGS
./bindiff -f e32-ref.dll -f e32-test.dll -i 0x14,0x18 -i 0x24,0x28
diff -u e32-ref.def e32-test.def
#rm e32-test.dll e32-ref.dll

