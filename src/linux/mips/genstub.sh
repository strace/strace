#!/bin/sh -e
#
# Copyright (c) 2015-2023 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

srcdir="$1"; shift
dstdir="$1"; shift

for n; do
	in="$srcdir/syscallent-$n.h"
	out="$dstdir/syscallent-$n-stub.h"
	sed -E -n '/^#if/,/^#else/ {s/^([^{]*\{[^,]*,[^,]*,[[:space:]]*)[^,[:space:]]+,[[:space:]]*"([^"]+".*)/\1SEN(printargs), SYSCALL_NAME_PREFIX "\2/; s/^\[.*/&/p}' < "$in" > "$out"
done
