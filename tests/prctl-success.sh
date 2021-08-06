#!/bin/sh -efu
#
# Check decoding of prctl ops using syscall injection.
#
# Expects a binary that accepts PRCTL_INJECT_START as the first argument.
# Accepts list of retvals to inject as first PRCTL_INJECT_RETVALS= argument
#
# Copyright (c) 2018-2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/scno_tampering.sh"

: ${PRCTL_INJECT_START=256}
: ${PRCTL_INJECT_RETVALS=42}

check_prog sed

# We avoid messing with arguments by accepting arguments we understand only
# at the beginning of the argument list.
while [ "$#" -gt 0 ]; do
	case "$1" in
	PRCTL_INJECT_RETVALS=*) PRCTL_INJECT_RETVALS="${1#PRCTL_INJECT_RETVALS=}"; ;;
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

for i in $(echo "$PRCTL_INJECT_RETVALS"); do
	run_strace -a80 "$@" -e trace=prctl \
		-e inject=prctl:retval="${i}":when="${PRCTL_INJECT_START}+" \
		"../$NAME" "${PRCTL_INJECT_START}" "${i}" > "$EXP.$i"
	sed '0,/^prctl(0xffffffff\( \/\* PR_??? \*\/\)\?, 0xfffffffe, 0xfffffffd, 0xfffffffc, 0xfffffffb) = '"$((i))"' /d' < "$LOG" > "$OUT.$i"
	match_diff "$OUT.$i" "$EXP.$i"
done
