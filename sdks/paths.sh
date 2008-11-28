#!/bin/sh

EKA1TOOLS=~/symbian-gcc/bin
EKA2TOOLS=~/csl-gcc/bin

if [ -f ${EPOCROOT}epoc32/tools/elf2e32.exe ]; then
  export PATH=${EKA2TOOLS}:${EPOCROOT}epoc32/tools:${PATH}
else
  export PATH=${EKA1TOOLS}:${EPOCROOT}epoc32/tools:${PATH}
fi

