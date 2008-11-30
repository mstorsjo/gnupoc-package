#!/bin/sh

. gnupoc-common.sh

TOOL=`basename $0`
if [ "`which $TOOL`" != "$0" ] && [ -x "`which $TOOL`" ]; then
	exec $TOOL "$@"
else
	exec wine ${EPOCROOT}epoc32/tools/$TOOL.exe "$@"
fi

