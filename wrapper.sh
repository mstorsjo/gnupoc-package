#!/bin/sh

if [ -f $EPOCROOT/epoc32/tools/elf2e32.exe ]; then
  export PATH=~/csl-gcc/bin:$EPOCROOT/epoc32/tools:$PATH
else
  export PATH=~/symbian-gcc/bin:$EPOCROOT/epoc32/tools:$PATH
fi

TOOL=`basename $0`
if [ "`which $TOOL`" != "$0" ] && [ -x "`which $TOOL`" ]; then
	exec $TOOL $*
else
	exec wine ${EPOCROOT}epoc32/tools/$TOOL.exe $*
fi

