#!/bin/sh

SIS=`echo $1 | sed s/pkg/sis/`
wine $EPOCROOT/epoc32/tools/makesis.exe $1 $SIS
DT=`../src/finddatetime $SIS`
md5sum $SIS >> md5sums
echo $'\t'"../src/makesis -t$DT $1" >> Makefile
mv $SIS $SIS.ref

