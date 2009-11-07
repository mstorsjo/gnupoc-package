#!/bin/sh

INPUT=$1

wine $EPOCROOT/epoc32/tools/getexports.exe $INPUT > out-ref.txt
./getexports $INPUT > out-test.txt
diff -uw out-ref.txt out-test.txt
rm -f out-ref.txt out-test.txt

