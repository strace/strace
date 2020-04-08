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

log_sfx()
{
	printf "%.128s" "$*" | tr -c '0-9A-Za-z-=,' '_'
}

check_exit_status_and_stderr()
{
	local sfx
	sfx="$1"; shift
	$STRACE "$@" 2> "$LOG.$sfx" && {
		cat "$LOG.$sfx" >&2
		fail_ "strace $* failed to handle the error properly"
	}
	match_diff "$LOG.$sfx" "$EXP.$sfx" \
		"strace $* failed to print expected diagnostics"
}

check_exit_status_and_stderr_using_grep()
{
	local sfx
	sfx="$1"; shift
	$STRACE "$@" 2> "$LOG.$sfx" && {
		cat "$LOG.$sfx" >&2
		fail_ "strace $* failed to handle the error properly"
	}
	match_grep "$LOG.$sfx" "$EXP.$sfx" \
		"strace $* failed to print expected diagnostics"
}

check_e()
{
	local pattern sfx
	pattern="$1"; shift
	sfx="$(log_sfx "$*")"
	cat > "$EXP.$sfx" << __EOF__
$STRACE_EXE: $pattern
__EOF__
	check_exit_status_and_stderr "$sfx" "$@"
}

check_e_using_grep()
{
	local pattern sfx
	pattern="$1"; shift
	sfx="$(log_sfx "$*")"
	cat > "$EXP.$sfx" << __EOF__
$STRACE_EXE: $pattern
__EOF__
	check_exit_status_and_stderr_using_grep "$sfx" "$@"
}

check_h()
{
	local patterns sfx
	patterns="$1"; shift
	sfx="$(log_sfx "$*")"
	{
		local pattern
		printf '%s\n' "$patterns" |
			while read -r pattern; do
				printf '%s: %s\n' "$STRACE_EXE" "$pattern"
			done
		printf "Try '%s -h' for more information.\\n" "$STRACE_EXE"
	} > "$EXP.$sfx"
	check_exit_status_and_stderr "$sfx" "$@"
}
