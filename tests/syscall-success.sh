#!/bin/sh -efu
#
# Check decoding of a syscall using syscall injection.
#
# Accepts a list of retvals to inject as the first INJECT_RETVALS= argument
# Accepts a syscall to inject retvals to as the first INJECT_SYSCALL= argument
#
# Copyright (c) 2018-2026 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/scno_tampering.sh"

: "${INJECT_RETVALS=42}"
: "${INJECT_SYSCALL=prctl}"

check_prog sed

# We avoid messing with arguments by accepting arguments we understand only
# at the beginning of the argument list.
while [ "$#" -gt 0 ]; do
	case "$1" in
	INJECT_RETVALS=*) INJECT_RETVALS="${1#INJECT_RETVALS=}" ;;
	INJECT_SYSCALL=*) INJECT_SYSCALL="${1#INJECT_SYSCALL=}" ;;
	*) break; ;;
	esac

	shift
done

"../$NAME" > /dev/null || {
	rc=$?
	case "$rc" in
		1) ;; # expected
		77) skip_ "../$NAME exited with code $rc" ;;
		*) fail_ "../$NAME failed with code $rc" ;;
	esac
}

for i in $(printf '%s' "$INJECT_RETVALS"); do
	run_strace -e "inject=${INJECT_SYSCALL}:retval=${i}" "$@" \
		"../$NAME" > "$EXP.$i"
	mv "$LOG" "$LOG.$i"
	match_diff "$LOG.$i" "$EXP.$i"
done
