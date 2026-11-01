#!/bin/sh -efu
#
# Shared test driver for poke_enter=argN=VAL injection tests.
# Queries the test binary for injection arguments, then runs strace.
#
# Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/scno_tampering.sh"

if [ -z "${STRACE_POKE_LEGACY-}" ]; then
	$STRACE -d -enone --fault=chdir / > /dev/null 2> "$LOG" ||:
	grep -qx "[^:]*strace: PTRACE_SET_SYSCALL_INFO works" "$LOG" ||
		skip_ 'PTRACE_SET_SYSCALL_INFO does not work'
fi

: "${NAME:=${0##*/}}"
NAME="${NAME%.gen.test}"

run_prog "../$NAME" > /dev/null
inject_args=$("../$NAME" --print-inject-args)
run_strace "--inject=splice:$inject_args" --trace=splice -a30 \
	"../$NAME" --assert > "$EXP"
match_diff "$LOG" "$EXP"
