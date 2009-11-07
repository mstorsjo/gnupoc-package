#!/bin/sh

INPUT=$1

wine $EPOCROOT/epoc32/tools/gendirective.exe $INPUT > out-ref.txt
./gendirective $INPUT > out-test.txt
diff -uw out-ref.txt out-test.txt
rm -f out-ref.txt out-test.txt

