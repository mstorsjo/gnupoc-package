#!/bin/sh

SIS=`echo $1 | sed s/pkg/ref.sis/`
wine $EPOCROOT/epoc32/tools/makesis.exe $1 $SIS
DT=`../../src/finddatetime $SIS`
md5sum $SIS | sed s/.ref// >> md5sums
echo $'\t'"../../src/makesis -t$DT $1" >> Makefile

