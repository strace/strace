#!/bin/sh -e

srcdir="${0%/*}"
dstdir="$1"; shift

for n in n32 n64 o32; do
	in="$srcdir/syscallent-$n.h"
	out="$dstdir/syscallent-$n-stub.h"
	sed -r -n '/^#if/,/^#else/ {s/^([^{]*\{[^,]*,[^,]*,[[:space:]]*)[^,[:space:]]+,[[:space:]]*"([^"]+".*)/\1SEN(printargs), "'$n':\2/; s/^\[.*/&/p}' < "$in" > "$out"
done
