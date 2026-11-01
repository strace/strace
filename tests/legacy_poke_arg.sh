#!/bin/sh -efu
#
# Check poke_enter=argN=VAL injection using legacy fallbacks
# when various PTRACE APIs are not available.
#
# Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/init.sh"

check_prog grep

check_helper()
{
	helper="$1"
	$helper true ||
		skip_ "$helper does not work"
}

NAME="${0##*/}"
NAME="${NAME%.gen.test}"
case "$NAME" in
	no_getregset_*)
		NAME="${NAME#no_getregset_}"
		[ "$STRACE_NATIVE_ARCH" = x86_64 ] ||
			skip_ 'PTRACE_GETREGS fallback needs an x86-64 tracer'
		check_helper '../../src/disable_ptrace_getregset'
		STRACE="$helper $STRACE"
		;;
	no_setregset_*)
		NAME="${NAME#no_setregset_}"
		[ "$STRACE_NATIVE_ARCH" = x86_64 ] ||
			skip_ 'PTRACE_SETREGSET fallback needs an x86-64 tracer'
		check_helper '../../src/disable_ptrace_setregset'
		STRACE="$helper $STRACE"
		;;
	no_getsi_*)
		NAME="${NAME#no_getsi_}"
		$STRACE -d -enone / > /dev/null 2> "$LOG" ||:
		if grep -qx "[^:]*strace: PTRACE_GET_SYSCALL_INFO works" "$LOG"; then
			check_helper '../../src/disable_ptrace_get_syscall_info'
			STRACE="$helper $STRACE"
			$STRACE -d -enone / > /dev/null 2> "$LOG" ||:
			grep -qx "[^:]*strace: PTRACE_GET_SYSCALL_INFO does not work" "$LOG" ||
				fail_ "$helper does not work properly"
		fi
		;;
	no_setsi_*)
		NAME="${NAME#no_setsi_}"
		;;
esac

$STRACE -d -enone --fault=chdir / > /dev/null 2> "$LOG" ||:
if grep -qx "[^:]*strace: PTRACE_SET_SYSCALL_INFO works" "$LOG"; then
	check_helper '../../src/disable_ptrace_set_syscall_info'
	STRACE="$helper $STRACE"
	$STRACE -d -enone --fault=chdir / > /dev/null 2> "$LOG" ||:
	grep -qx "[^:]*strace: PTRACE_SET_SYSCALL_INFO does not work" "$LOG" ||
		fail_ "$helper does not work properly"
fi

case "$STRACE_ARCH" in
	ia64)
		skip_ 'setting syscall arguments on ia64 requires PTRACE_SET_SYSCALL_INFO'
		;;
	arc|riscv64)
		case "$NAME" in
			poke_arg_a1*)
				. "${srcdir=.}/scno_tampering.sh"
				run_prog "../$NAME" > /dev/null
				inject_args=$("../$NAME" --print-inject-args)
				> "$LOG" || framework_failure_
				$STRACE -o "$LOG" \
					"--inject=splice:$inject_args" \
					--trace=splice -a30 \
					"../$NAME" > /dev/null 2> "${LOG}.err"
				grep -F 'Failed to tamper' "${LOG}.err" > /dev/null ||
					dump_log_and_fail_with \
						"expected 'Failed to tamper' error"
				grep -F 'INJECTED' "$LOG" > /dev/null &&
					dump_log_and_fail_with \
						"injection should not have succeeded on $STRACE_ARCH"
				exit 0
				;;
		esac
		;;
esac

STRACE_POKE_LEGACY=1
. "${srcdir=.}/$NAME.gen.test"
