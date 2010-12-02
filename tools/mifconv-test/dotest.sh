#!/bin/sh

# EPOCROOT needs to be set to an S60 3.0 SDK (which doesn't do svgtbinencode)

# Testing with bmp files requires bmconv.exe to be placed in the wine
# path, e.g. in ~/.wine/drive_c/windows
export PATH=../bmconv-1.1.0-2/src:$PATH

echo Testing $*
wine $EPOCROOT/epoc32/tools/mifconv.exe ref.mif /hheader.mbg $*
mv header.mbg header.mbg.ref
../mifconv test.mif /hheader.mbg $*
diff -u header.mbg.ref header.mbg || exit 1
cmp test.mif ref.mif || exit 1
(echo $* | grep bmp > /dev/null) || exit 0
cmp test.mbm ref.mbm || exit 1

