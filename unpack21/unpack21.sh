#!/bin/sh

if [ $0 != "unpack21/unpack21.sh" ] ; then
  echo "must be run as unpack21/unpack21.sh"
  echo "current directory must contain S60_SDK_2_1_NET.zip"
  exit 1;
fi
mkdir unpack21_tmp

echo "  Unzipping the SDK archive"
# Unpacking the Nokia S60 SDK...
unzip -qd unpack21_tmp/tmp21/ $1
if test $? != 0; then
  exit 1
fi

echo "  Extracting the CAB files"
mkdir unpack21_tmp/data1_files
cabextract -q -dunpack21_tmp/data1_files "unpack21_tmp/tmp21/126 Series 60 v2.1 Final DOT NET 03.06.2004 0840/Data1.cab"

echo "  Creating directory structure"
mkdir target
cat unpack21/dirs.txt | xargs -n 1 mkdir

echo "  Un-obfuscating file names and moving into place"
cat unpack21/msname_realpath.txt | xargs -n 2 cp
rm -rf unpack21_tmp
