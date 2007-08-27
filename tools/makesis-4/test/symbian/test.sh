#!/bin/sh

wine $EPOCROOT/epoc32/tools/makesis.exe $1 ref.sis
../../src/dumpcontroller ref.sis ref.controller
../../src/dumptree ref.controller 0 > ref.controller.tree
DT=`../../src/finddatetime ref.sis`
../../src/makesis -t$DT $1 test.sis  || exit 1
../../src/dumpcontroller test.sis test.controller
../../src/dumptree test.controller 0 > test.controller.tree
diff -u ref.controller.tree test.controller.tree || exit 1
../../../elf2e32/bindiff ref.controller test.controller || echo controller && exit 1
../../../elf2e32/bindiff ref.sis test.sis || echo sis && exit 1

