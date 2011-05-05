#!/bin/sh

if [ $# -lt 1 ]; then
	echo $0 qtdir
	exit 1
fi

cd $1
if [ ! -d include ] || [ ! -d src ]; then
	echo The include and/or src dir is missing
	exit 1
fi

grep -r include include/* | grep ../src | while read line; do
	file=`echo $line | cut -d : -f 1`
	orig=`echo $line | sed 's/.*#include "\([^"]*\)".*/\1/'`
	origfile=`dirname $file`/$orig
	cp $origfile $file
done

