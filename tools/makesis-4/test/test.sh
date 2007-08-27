#!/bin/sh

wine $EPOCROOT/epoc32/tools/makesis.exe $1 ref.sis
../src/dumpcontroller ref.sis ref.controller
DT=`../src/finddatetime ref.sis`
../src/makesis -t$DT $1 test.sis  || exit 1
../src/dumpcontroller test.sis test.controller
../../elf2e32/bindiff ref.controller test.controller
../../elf2e32/bindiff ref.sis test.sis

