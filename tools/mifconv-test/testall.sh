#!/bin/sh

./dotest.sh /c24 redrect.svg || exit 1
./dotest.sh /c32 redrect.svg /c24 greenrect.svg /c16 bluerect.svg || exit 1
./dotest.sh /c12 redrect.svg /c8 greenrect.svg /c4 bluerect.svg || exit 1
./dotest.sh /8 redrect.svg /4 greenrect.svg /2 bluerect.svg || exit 1
./dotest.sh /1 redrect.svg /1,1 greenrect.svg /1,8 bluerect.svg || exit 1
./dotest.sh /c32,1 redrect.svg /c24,8 greenrect.svg /a /c24 bluerect.svg || exit 1

