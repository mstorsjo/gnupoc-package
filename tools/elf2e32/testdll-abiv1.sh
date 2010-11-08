#!/bin/sh

INPUT=$1

ARGS="-nocompress -sid 0x00000000 -uid1 0x1000007a -uid2 0x00000000 -uid3 0x00000000 -vid 0x00000000 -capability none -fpu softvfp -version 4.2 $INPUT -allowdlldata"

wine $EPOCROOT/epoc32/tools/elftran.exe $ARGS e32-ref.dll
./elftran $ARGS e32-test.dll
./bindiff -i 0x14,0x18 -i 0x24,0x28 e32-ref.dll e32-test.dll && exit 0
wine $EPOCROOT/epoc32/tools/elftran.exe e32-ref.dll > e32-ref-dump.txt
wine $EPOCROOT/epoc32/tools/elftran.exe e32-test.dll > e32-test-dump.txt
#rm e32-test.dll e32-ref.dll

exit 1

