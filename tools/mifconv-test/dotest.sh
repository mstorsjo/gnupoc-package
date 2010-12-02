#!/bin/sh

# EPOCROOT needs to be set to an S60 3.0 SDK (which doesn't do svgtbinencode)

echo Testing $*
wine $EPOCROOT/epoc32/tools/mifconv.exe ref.mif /hheader.mbg $*
mv header.mbg header.mbg.ref
../mifconv test.mif /hheader.mbg $*
diff -u header.mbg.ref header.mbg || exit 1
cmp test.mif ref.mif || exit 1

