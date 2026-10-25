#!/bin/sh -efu
#
# Check that explicit $CLONE injection produces INJECTED annotation.
#
# Copyright (c) 2026 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/scno_tampering.sh"
. "${srcdir=.}/filter_seccomp.sh"

check_prog grep

if [ -z "${STRACE_POKE_LEGACY-}" ]; then
	$STRACE -d -enone --fault=chdir / > /dev/null 2> "$LOG" ||:
	grep -qx "[^:]*strace: PTRACE_SET_SYSCALL_INFO works" "$LOG" ||
		skip_ 'PTRACE_SET_SYSCALL_INFO does not work'
fi

: "${NAME:=${0##*/}}"
NAME="${NAME%.gen.test}"
NAME="${NAME%-inject}"
CLONE="${NAME%_untraced}"
WHEN=
case "$CLONE" in
	clone) ;;
	clone3) WHEN=':when=2' ;;
	*) framework_failure_ 'who am I?' ;;
esac

run_prog "../$NAME"
run_strace --seccomp-bpf -f -e trace="$CLONE,chdir" \
	-e inject="$CLONE$WHEN:error=EISDIR" ../$NAME

grep -qF 'INJECTED' "$LOG" ||
	dump_log_and_fail_with "explicit $CLONE injection was not applied"

grep -qF "chdir(\"$CLONE skipped\"" "$LOG" ||
	dump_log_and_fail_with "$CLONE injection did not reach the program"
