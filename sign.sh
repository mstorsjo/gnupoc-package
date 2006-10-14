#!/bin/bash
if [ $# = 4 ] ; then
	echo "Enter password:"
	read -e -s pwd
	signsis $1 $2 $3 $4 $pwd
else
	echo "$0 sisfile sisxfile cert key"
fi

