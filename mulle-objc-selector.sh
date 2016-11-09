#! /bin/sh

if [ "$1" = "" ]
then
   echo "usage: mulle-objc-selector.sh <selector>" >&2
   exit 1
fi

echo "0x"`md5 -q -s "$1" | cut -c1-16`


