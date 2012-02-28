#!/bin/sh

grep '^#define' dummy.h | cut -f2 | \
while read func; do
	grep -q -F -- "${func}(" syscall.h && echo "Defined as macro and as func: $func"
done
