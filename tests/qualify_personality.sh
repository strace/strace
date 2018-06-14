#! /bin/sh -efu
#
# Common code for per-personality qualification tests
#
# Copyright (c) 2018 The strace developers.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
aarch64|powerpc64|riscv|s390x|sparc64|tile)
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
