#!/bin/sh
#
# Copyright (c) 2011-2016 Dmitry V. Levin <ldv@altlinux.org>
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

ME_="${0##*/}"
LOG="$ME_.tmp"
OUT="$LOG.out"
EXP="$LOG.exp"
NAME="${ME_%.test}"

warn_() { printf >&2 '%s\n' "$*"; }
fail_() { warn_ "$ME_: failed test: $*"; exit 1; }
skip_() { warn_ "$ME_: skipped test: $*"; exit 77; }
framework_failure_() { warn_ "$ME_: framework failure: $*"; exit 99; }
framework_skip_() { warn_ "$ME_: framework skip: $*"; exit 77; }

check_prog()
{
	type "$@" > /dev/null 2>&1 ||
		framework_skip_ "$* is not available"
}

dump_log_and_fail_with()
{
	cat < "$LOG"
	fail_ "$*"
}

run_prog()
{
	if [ $# -eq 0 ]; then
		set -- "./$NAME"
	fi
	args="$*"
	"$@" || {
		rc=$?
		if [ $rc -eq 77 ]; then
			skip_ "$args exited with code 77"
		else
			fail_ "$args failed with code $rc"
		fi
	}
}


run_prog_skip_if_failed()
{
	args="$*"
	"$@" || framework_skip_ "$args failed with code $?"
}

run_strace()
{
	> "$LOG" || fail_ "failed to write $LOG"
	args="$*"
	$STRACE -o "$LOG" "$@" ||
		dump_log_and_fail_with "$STRACE $args failed with code $?"
}

run_strace_merge()
{
	rm -f -- "$LOG".[0-9]*
	run_strace -ff -tt "$@"
	"$srcdir"/../strace-log-merge "$LOG" > "$LOG" ||
		dump_log_and_fail_with 'strace-log-merge failed with code $?'
	rm -f -- "$LOG".[0-9]*
}

check_gawk()
{
	check_prog gawk
	check_prog grep

	local program="$1"; shift
	if grep '^@include[[:space:]]' < "$program" > /dev/null; then
		gawk '@include "/dev/null"' < /dev/null ||
			framework_skip_ 'gawk does not support @include'
	fi
}

# Usage: [FILE_TO_CHECK [AWK_PROGRAM [ERROR_MESSAGE [EXTRA_AWK_OPTIONS...]]]]
# Check whether AWK_PROGRAM matches FILE_TO_CHECK using gawk.
# If it doesn't, dump FILE_TO_CHECK and fail with ERROR_MESSAGE.
match_awk()
{
	local output program error
	if [ $# -eq 0 ]; then
		output="$LOG"
	else
		output="$1"; shift
	fi
	if [ $# -eq 0 ]; then
		program="$srcdir/$NAME.awk"
	else
		program="$1"; shift
	fi
	if [ $# -eq 0 ]; then
		error="$STRACE $args output mismatch"
	else
		error="$1"; shift
	fi

	check_gawk "$program"

	AWKPATH="$srcdir" gawk -f "$program" "$@" < "$output" || {
		cat < "$output"
		fail_ "$error"
	}
}

# Usage: [FILE_TO_CHECK [FILE_TO_COMPATE_WITH [ERROR_MESSAGE]]]
# Check whether FILE_TO_CHECK differs from FILE_TO_COMPATE_WITH.
# If it does, dump the difference and fail with ERROR_MESSAGE.
match_diff()
{
	local output expected error
	if [ $# -eq 0 ]; then
		output="$LOG"
	else
		output="$1"; shift
	fi
	if [ $# -eq 0 ]; then
		expected="$srcdir/$NAME.expected"
	else
		expected="$1"; shift
	fi
	if [ $# -eq 0 ]; then
		error="$STRACE $args output mismatch"
	else
		error="$1"; shift
	fi

	check_prog diff

	diff -- "$expected" "$output" ||
		fail_ "$error"
}

# Usage: [FILE_TO_CHECK [FILE_WITH_PATTERNS [ERROR_MESSAGE]]]
# Check whether all patterns listed in FILE_WITH_PATTERNS
# match FILE_TO_CHECK using egrep.
# If at least one of these patterns does not match,
# dump both files and fail with ERROR_MESSAGE.
match_grep()
{
	local output patterns error pattern failed=
	if [ $# -eq 0 ]; then
		output="$LOG"
	else
		output="$1"; shift
	fi
	if [ $# -eq 0 ]; then
		patterns="$srcdir/$NAME.expected"
	else
		patterns="$1"; shift
	fi
	if [ $# -eq 0 ]; then
		error="$STRACE $args output mismatch"
	else
		error="$1"; shift
	fi

	check_prog wc
	check_prog grep

	while read -r pattern; do
		LC_ALL=C grep -E -x -e "$pattern" < "$output" > /dev/null || {
			test -n "$failed" || {
				echo 'Failed patterns of expected output:'
				failed=1
			}
			printf '%s\n' "$pattern"
		}
	done < "$patterns"
	test -z "$failed" || {
		echo 'Actual output:'
		cat < "$output"
		fail_ "$error"
	}
}

# Usage: run_strace_match_diff [args to run_strace]
run_strace_match_diff()
{
	args="$*"
	[ -n "$args" -a -z "${args##*-e trace=*}" ] ||
		set -- -e trace="$NAME" "$@"
	run_prog > /dev/null
	run_strace "$@" $args > "$EXP"
	match_diff "$LOG" "$EXP"
	rm -f "$EXP"
}

check_prog cat
check_prog rm

rm -f "$LOG"

: "${STRACE:=../strace}"
: "${TIMEOUT_DURATION:=60}"
: "${SLEEP_A_BIT:=sleep 1}"

[ -z "${VERBOSE-}" ] ||
	set -x
