#!/bin/sh

for i in testcases/*.exe; do
	echo $i
	./test.sh $i
done
for i in testcases/*.dll; do
	echo $i
	./testdll.sh $i
done

