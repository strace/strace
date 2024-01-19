#! /bin/sh -efu
#
# Common code for per-personality qualification tests
#
# Copyright (c) 2018-2023 The strace developers.
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
	supported_pers="$((SIZEOF_LONG * 8))"
	;;
esac

cur_pers=$(print_current_personality_designator)

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
