#! /bin/sh -efu
#
# Common code for per-personality qualification tests
#
# Copyright (c) 2018-2019 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/init.sh"

[ 2 -le "$#" ] ||
	fail_ 'No personality designation ("64", "32", "x32") specified'

pers="$1"
shift
trace_expr="$1"
shift
skip="${1-}"

case "$STRACE_NATIVE_ARCH" in
x86_64)
	supported_pers='64 32 x32'
	;;
x32)
	supported_pers='x32 32'
	;;
aarch64|powerpc64|s390x|sparc64|tile)
	supported_pers='64 32'
	;;
*)
	supported_pers="$(($SIZEOF_LONG * 8))"
	;;
esac

# Detect current personality designation
if [ "x$STRACE_NATIVE_ARCH" = "x$STRACE_ARCH" ]; then
	case "$STRACE_NATIVE_ARCH" in
	x32)
		cur_pers=x32
		;;
	*)
		cur_pers="$(($SIZEOF_LONG * 8))"
		;;
	esac
else
	if [ "x$SIZEOF_KERNEL_LONG_T" = "x$SIZEOF_LONG" ]; then
		[ 4 -eq "$SIZEOF_LONG" ] ||
			fail_ "sizeof(long) = $SIZEOF_LONG != 4"
		cur_pers=32
	else
		[ 8 -eq "$SIZEOF_KERNEL_LONG_T" ] ||
			fail_ "sizeof(kernel_long_t) = $SIZEOF_KERNEL_LONG_T != 8"
		[ 4 -eq "$SIZEOF_LONG" ] ||
			fail_ "sizeof(long) = $SIZEOF_LONG != 4"
		cur_pers=x32
	fi
fi

pers_found=0
set -- $supported_pers
for i; do
	[ "x$pers" != "x$i" ] || pers_found=1
done

[ "$pers_found" = 1 ] ||
	skip_ "Personality '$pers' is not supported on architecture" \
	      "'$STRACE_NATIVE_ARCH' (supported personalities: $supported_pers)"

# If tested personality is not equivalent to current personality, reset $NAME,
# so "$NAME.in", which is used by test_trace_expr, points to an empty file.
[ "x$pers" = "x$cur_pers" ] || NAME=qualify_personality_empty

test_trace_expr "$skip" -e trace="${trace_expr}@${pers}"
