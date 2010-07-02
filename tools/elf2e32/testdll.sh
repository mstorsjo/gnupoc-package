#!/bin/sh

INPUT=$1

ARGS="--uncompressed --sid=0x00000000 --uid1=0x10000079 --uid2=0x1000008d --uid3=0x00000000 --vid=0x00000000 --capability=none --fpu=softvfp --targettype=DLL --elfinput=$INPUT --linkas=libname{000a0000}[00000000].dll --libpath=$EPOCROOT/epoc32/release/armv5/lib/"

wine $EPOCROOT/epoc32/tools/elf2e32.exe --output=e32-ref.dll --defoutput=e32-ref.def --dso=e32-ref.dso --uncompressed $ARGS 
wine $EPOCROOT/epoc32/tools/elf2e32.exe --e32input=e32-ref.dll > e32info.txt
mv e32-ref.dso e32-ref.dso.orig
# The dso name is embedded in the file; use the same name to ease diffing
./elf2e32 --output=e32-test.dll --defoutput=e32-test.def --dso=e32-ref.dso $ARGS
mv e32-ref.dso e32-test.dso
mv e32-ref.dso.orig e32-ref.dso
./bindiff -i 0x14,0x18 -i 0x24,0x28 e32-ref.dll e32-test.dll
diff -u e32-ref.def e32-test.def
wine $EPOCROOT/epoc32/tools/elf2e32.exe --e32input=e32-test.dll > e32info-test.txt
rm e32-test.dll e32-ref.dll

compare() {
	CMD=$1
	ARGS=$2
	$CMD $ARGS e32-ref.dso > ref-out.txt
	$CMD $ARGS e32-test.dso | sed s/e32-test.dso/e32-ref.dso/ > test-out.txt
	diff -u ref-out.txt test-out.txt
}

# objdump -s produces differences in .dynamic and .hash (and in .version sometimes)
compare arm-none-symbianelf-objdump "-s -j ER_RO -j .version_d -j .dynsym -j .strtab"
compare arm-none-symbianelf-readelf -sW 2>/dev/null
compare arm-none-symbianelf-objdump -r
compare arm-none-symbianelf-objdump -t

