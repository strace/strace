#!/bin/sh

for i in ../*.h; do
	NM=`basename $i .h`1.h
	/bin/cp -vf $i $NM
done

patch -p0 < i386-headers.diff
