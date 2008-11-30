#!/bin/sh

EKA1TOOLS=~/symbian-gcc/bin
EKA2TOOLS=~/csl-gcc/bin

if [ -f ${EPOCROOT}epoc32/tools/elf2e32.exe ]; then
	export PATH=${EKA2TOOLS}:${EPOCROOT}epoc32/tools:${PATH}
else
	export PATH=${EKA1TOOLS}:${EPOCROOT}epoc32/tools:${PATH}
fi

# winc tools (such as cshlpwtr) use hal.dll from epoc which
# shouldn't be overridden by the ordinary windows hal.dll
# provided by wine
OVERRIDES="hal=n"

if [ -z "$WINEDLLOVERRIDES" ]; then
	export WINEDLLOVERRIDES="$OVERRIDES"
else
	export WINEDLLOVERRIDES="$OVERRIDES;$WINEDLLOVERRIDES"
fi

