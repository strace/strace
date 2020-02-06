#!/bin/sh
#
# Define syntax testing primitives.
#
# Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
# Copyright (c) 2016-2020 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/init.sh"

check_exit_status_and_stderr()
{
	$STRACE "$@" 2> "$LOG" &&
		dump_log_and_fail_with \
			"strace $* failed to handle the error properly"
	match_diff "$LOG" "$EXP" \
		"strace $* failed to print expected diagnostics"
}

check_exit_status_and_stderr_using_grep()
{
	$STRACE "$@" 2> "$LOG" &&
		dump_log_and_fail_with \
			"strace $* failed to handle the error properly"
	match_grep "$LOG" "$EXP" \
		"strace $* failed to print expected diagnostics"
}

check_e()
{
	local pattern="$1"; shift
	cat > "$EXP" << __EOF__
$STRACE_EXE: $pattern
__EOF__
	check_exit_status_and_stderr "$@"
}

check_e_using_grep()
{
	local pattern="$1"; shift
	cat > "$EXP" << __EOF__
$STRACE_EXE: $pattern
__EOF__
	check_exit_status_and_stderr_using_grep "$@"
}

check_h()
{
	local patterns="$1"; shift
	{
		local pattern
		printf '%s\n' "$patterns" |
			while read -r pattern; do
				printf '%s: %s\n' "$STRACE_EXE" "$pattern"
			done
		printf "Try '%s -h' for more information.\\n" "$STRACE_EXE"
	} > "$EXP"
	check_exit_status_and_stderr "$@"
}
