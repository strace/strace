#!/bin/sh -efu
#
# Check decoding of prctl ops using syscall injection.
#
# Expects a binary that accepts PRCTL_INJECT_START as the first argument.
# Accepts list of retvals to inject as first PRCTL_INJECT_RETVALS= argument
#
# Copyright (c) 2018-2024 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/scno_tampering.sh"

: "${PRCTL_INJECT_START=256}"
: "${PRCTL_INJECT_RETVALS=42}"
: "${PRCTL_SYSCALL=prctl}"
: "${PRCTL_MARKER_RE='prctl(0xffffffff\( \/\* PR_??? \*\/\)\?, 0xfffffffe, 0xfffffffd, 0xfffffffc, 0xfffffffb)'}"

check_prog sed

# We avoid messing with arguments by accepting arguments we understand only
# at the beginning of the argument list.
while [ "$#" -gt 0 ]; do
	case "$1" in
	ARCH_PRCTL_INJECT_RETVALS=*)
		PRCTL_INJECT_RETVALS="${1#ARCH_PRCTL_INJECT_RETVALS=}"
		PRCTL_SYSCALL=arch_prctl
		PRCTL_MARKER_RE='arch_prctl(0xffffffff\( \/\* ARCH_??? \*\/\)\?, 0xfffffffe)'
		;;
	PRCTL_INJECT_RETVALS=*)
		PRCTL_INJECT_RETVALS="${1#PRCTL_INJECT_RETVALS=}"
		PRCTL_SYSCALL=prctl
		PRCTL_MARKER_RE='prctl(0xffffffff\( \/\* PR_??? \*\/\)\?, 0xfffffffe, 0xfffffffd, 0xfffffffc, 0xfffffffb)'
		;;
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
	if [ "x${i}" = "x${i#error=}" ]; then
		inj_str="retval=$((i))"
		ret_val="${i}"
		sed_match="$((i))"
	else
		inj_str="${i}"
		ret_val="-1"
		sed_match="-1 ${i#error=}"
	fi

	run_strace -a80 "$@" -e trace="${PRCTL_SYSCALL}" \
		-e "inject=${PRCTL_SYSCALL}:${inj_str}:when=${PRCTL_INJECT_START}+" \
		"../$NAME" "${PRCTL_INJECT_START}" "${ret_val}" > "$EXP.$i"
	sed '0,/^'"${PRCTL_MARKER_RE}"'  *= '"${sed_match}"' /d' < "$LOG" > "$OUT.$i"
	match_diff "$OUT.$i" "$EXP.$i"
done
