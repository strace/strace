#!/bin/sh
#
# Copyright (c) 2011-2016 Dmitry V. Levin <ldv@altlinux.org>
# Copyright (c) 2011-2020 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

export LC_ALL=C
ME_="${0##*/}"
LOG="log"
OUT="out"
EXP="exp"
CONFIG_H="../../config.h"

warn_() { printf >&2 '%s\n' "$*"; }
fail_() { warn_ "$ME_: failed test: $*"; exit 1; }
skip_() { warn_ "$ME_: skipped test: $*"; exit 77; }
framework_failure_() { warn_ "$ME_: framework failure: $*"; exit 99; }
framework_skip_() { warn_ "$ME_: framework skip: $*"; exit 77; }

# get_config_str OPTION
#
# Returns the value of OPTION from config.h (path to which set
# in the CONFIG_H variable).
get_config_str()
{
	sed -r -n 's/#define[[:space:]]*'"$1"'[[:space:]]*"([^"]*)".*/\1/p' \
		"$CONFIG_H"
}

# get_config_option OPTION YES_STRING [NO_STRING]
#
# Returns YES_STRING in case OPTION is enabled (present in config.h and has
# a non-zero numeric value). Otherwise, NO_STRING (or empty string, if not
# specified) is returned.
get_config_option()
{
	local opt
	opt=$(sed -r -n 's/#define[[:space:]]*'"$1"'[[:space:]]*([0-9]+)$/\1/p' \
		"$CONFIG_H")
	if [ -n "$opt" -a "$opt" -ne 0 ]; then
		printf "%s" "$2"
	else
		printf "%s" "${3-}"
	fi
}

check_prog()
{
	type "$@" > /dev/null 2>&1 ||
		framework_skip_ "$* is not available"
}

dump_log_and_fail_with()
{
	cat < "$LOG" >&2
	fail_ "$*"
}

run_prog()
{
	if [ $# -eq 0 ]; then
		set -- "../$NAME"
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

try_run_prog()
{
	local rc

	"$@" > /dev/null || {
		rc=$?
		if [ $rc -eq 77 ]; then
			return 1
		else
			fail_ "$* failed with code $rc"
		fi
	}
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

	diff -u -- "$expected" "$output" ||
		fail_ "$error"
}

# Usage: [FILE_TO_CHECK [FILE_WITH_PATTERNS [ERROR_MESSAGE]]]
# Check whether all patterns listed in FILE_WITH_PATTERNS
# match FILE_TO_CHECK using egrep.
# If at least one of these patterns does not match,
# dump both files and fail with ERROR_MESSAGE.
match_grep()
{
	local output patterns error pattern cnt failed= rc negated
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

	cnt=1
	while read -r pattern; do
		negated=0
		[ "x${pattern#!}" = "x${pattern}" ] ||
			negated=1

		rc="$negated"
		LC_ALL=C grep -E -x -e "${pattern#!}" < "$output" > /dev/null ||
			rc="$((!negated))"

		[ 0 = "$rc" ] || {
			test -n "$failed" || {
				echo 'Failed patterns of expected output:'
				failed=1
			}
			printf '#%d: %s\n' "$cnt" "$pattern"
		}
		cnt=$(($cnt + 1))
	done < "$patterns" ||
		fail_ "Error reading patterns from \"$patterns\""
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
	[ -n "$args" -a \( -z "${args##*-e trace=*}" -o -z "${args##*--trace=*}" \) ] ||
		set -- -e trace="$NAME" "$@"
	run_prog > /dev/null
	run_strace "$@" $args > "$EXP"
	match_diff "$LOG" "$EXP"
}

# Usage: run_strace_match_grep [args to run_strace]
run_strace_match_grep()
{
	args="$*"
	[ -n "$args" -a \( -z "${args##*-e trace=*}" -o -z "${args##*--trace=*}" \) ] ||
		set -- -e trace="$NAME" "$@"
	run_prog > /dev/null
	run_strace "$@" $args > "$EXP"
	match_grep "$LOG" "$EXP"
}

# Print kernel version code.
# usage: kernel_version_code $(uname -r)
kernel_version_code()
{
	(
		set -f
		IFS=.
		set -- $1 0 0
		v1="${1%%[!0-9]*}" && [ -n "$v1" ] || v1=0
		v2="${2%%[!0-9]*}" && [ -n "$v2" ] || v2=0
		v3="${3%%[!0-9]*}" && [ -n "$v3" ] || v3=0
		echo "$(($v1 * 65536 + $v2 * 256 + $v3))"
	)
}

# Usage: require_min_kernel_version_or_skip 3.0
require_min_kernel_version_or_skip()
{
	local uname_r
	uname_r="$(uname -r)"

	[ "$(kernel_version_code "$uname_r")" -ge \
	  "$(kernel_version_code "$1")" ] ||
		skip_ "the kernel release $uname_r is not $1 or newer"
}

# Usage: require_min_nproc 2
require_min_nproc()
{
	local min_nproc
	min_nproc="$1"; shift

	check_prog nproc
	local nproc
	nproc="$(nproc)"

	[ "$nproc" -ge "$min_nproc" ] ||
		framework_skip_ "nproc = $nproc is less than $min_nproc"
}

# Usage: grep_pid_status $pid GREP-OPTIONS...
grep_pid_status()
{
	local pid
	pid=$1; shift
	cat < "/proc/$pid/status" | grep "$@"
}

# Subtracts one program set from another.
# If an optional regular expression is specified, the lines in the minuend file
# that match this regular expression are elso excluded from the output.
#
# Usage: prog_set_subtract minuend_file subtrahend_file [subtrahend_regexp]
prog_set_subtract()
{
	local min sub re pat
	min="$1"; shift
	sub="$1"; shift
	re="${1-}"
	pat="$re|$(sed 's/[[:space:]].*//' < "$sub" | tr -s '\n' '|')"
	grep -E -v -x -e "$pat" < "$min"
}

# Usage: test_pure_prog_set [--expfile FILE] COMMON_ARGS < tests_file
# stdin should consist of lines in "test_name strace_args..." format.
test_pure_prog_set()
{
	local expfile

	expfile="$EXP"

	while [ -n "$1" ]; do
		case "$1" in
		--expfile)
			shift
			expfile="$1"
			shift
			;;
		*)
			break
			;;
		esac
	done

	while read -r t prog_args; do {
		# skip lines beginning with "#" symbol
		[ "${t###}" = "$t" ] || continue

		try_run_prog "../$t" || continue
		run_strace $prog_args "$@" "../$t" > "$expfile"

		case "$STRACE_ARCH:$MIPS_ABI:$NAME" in
			mips:o32:*creds)
				sed -i '/^prctl(PR_GET_FP_MODE)  *= 0$/d' "$LOG"
				;;
		esac

		match_diff "$LOG" "$expfile"
	} < /dev/null; done
}

# Run strace against list of programs put in "$NAME.in" and then against the
# rest of pure_executables.list with the expectation of empty output in the
# latter case.
#
# Usage: source this file after init.sh and call:
#   test_trace_expr subtrahend_regexp strace_args
# Environment:
#   $NAME:	test name, used for "$NAME.in" file containing list of tests
#		for positive trace expression match;
#   $srcdir:	used to find pure_executables.list and "$NAME.in" files.
# Files created:
#   negative.list: File containing list of tests for negative match.
test_trace_expr()
{
	local subtrahend_regexp
	subtrahend_regexp="$1"; shift
	test_pure_prog_set "$@" < "$srcdir/$NAME.in"
	prog_set_subtract "$srcdir/pure_executables.list" "$srcdir/$NAME.in" \
		"$subtrahend_regexp" > negative.list
	test_pure_prog_set --expfile /dev/null -qq -esignal=none "$@" \
		< negative.list
}

test_prog_set()
{
	test_pure_prog_set "$@" < "$srcdir/$NAME.in"
}

test_pidns_run_strace()
{
	local parent_pid init_pid

	check_prog tail
	check_prog cut
	check_prog grep

	run_prog > /dev/null
	run_strace --pidns-translation -f $@ $args > "$EXP"

	# filter out logs made by the parent or init process of the pidns test
	parent_pid="$(tail -n 2 $LOG | head -n 1 | cut -d' ' -f1)"
	init_pid="$(tail -n 1 $LOG | cut -d' ' -f1)"
	grep -E -v "^($parent_pid|$init_pid) " "$LOG" > "$OUT"
	match_diff "$OUT" "$EXP"
}

test_pidns()
{
	check_prog unshare
	unshare -Urpf true || framework_skip_ "unshare -Urpf true failed"

	test_pidns_run_strace "$@"

	# test PID translation when /proc is mounted from an other namespace
	STRACE="unshare -Urpf $STRACE"
	test_pidns_run_strace "$@"
}

check_prog cat
check_prog rm

case "$ME_" in
	*.gen.test) NAME="${ME_%.gen.test}" ;;
	*.test) NAME="${ME_%.test}" ;;
	*) NAME=
esac

STRACE_EXE=
if [ -n "$NAME" ]; then
	TESTDIR="$NAME.dir"
	rm -rf -- "$TESTDIR"
	mkdir -- "$TESTDIR"
	cd "$TESTDIR"

	case "$srcdir" in
		/*) ;;
		*) srcdir="../$srcdir" ;;
	esac

	[ -n "${STRACE-}" ] || {
		STRACE=../../strace
		case "${LOG_COMPILER-} ${LOG_FLAGS-}" in
			*--suppressions=*--error-exitcode=*--tool=*)
			STRACE_EXE="$STRACE"
			# add valgrind command prefix
			STRACE="${LOG_COMPILER-} ${LOG_FLAGS-} $STRACE"
			;;
		esac
	}

	trap 'dump_log_and_fail_with "time limit ($TIMEOUT_DURATION) exceeded"' XCPU
else
	: "${STRACE:=../strace}"
fi

# Export $STRACE_EXE to check_PROGRAMS.
: "${STRACE_EXE:=$STRACE}"
export STRACE_EXE

: "${TIMEOUT_DURATION:=1500}"
: "${SLEEP_A_BIT:=sleep 1}"

[ -z "${VERBOSE-}" ] ||
	set -x
