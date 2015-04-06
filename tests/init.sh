#!/bin/sh

ME_="${0##*/}"

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
		set -- "./${ME_%.test}"
	fi
	args="$*"
	"$@" || {
		if [ $? -eq 77 ]; then
			skip_ "$args exited with code 77"
		else
			fail_ "$args failed"
		fi
	}
}


run_prog_skip_if_failed()
{
	args="$*"
	"$@" || framework_skip_ "$args failed"
}

run_strace()
{
	> "$LOG" || fail_ "failed to write $LOG"
	args="$*"
	$STRACE -o "$LOG" "$@" ||
		dump_log_and_fail_with "$STRACE $args failed"
}

run_strace_merge()
{
	rm -f -- "$LOG".[0-9]*
	run_strace -ff -tt "$@"
	"$srcdir"/../strace-log-merge "$LOG" > "$LOG" ||
		dump_log_and_fail_with 'strace-log-merge failed'
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
# Check whether all patterns listed in AWK_PROGRAM
# match FILE_TO_CHECK using egrep.
# If at least one of these patterns does not match,
# dump both files and fail with ERROR_MESSAGE.
match_awk()
{
	local output program error
	if [ $# -eq 0 ]; then
		output="$LOG"
	else
		output="$1"; shift
	fi
	if [ $# -eq 0 ]; then
		program="$srcdir/${ME_%.test}.awk"
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
		expected="$srcdir/${ME_%.test}.expected"
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
	local output patterns error expected matched
	if [ $# -eq 0 ]; then
		output="$LOG"
	else
		output="$1"; shift
	fi
	if [ $# -eq 0 ]; then
		patterns="$srcdir/${ME_%.test}.expected"
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

	expected=$(wc -l < "$patterns") &&
	matched=$(LC_ALL=C grep -c -E -x -f "$patterns" < "$output") &&
	test "$expected" -eq "$matched" || {
		echo 'Patterns of expected output:'
		cat < "$patterns"
		echo 'Actual output:'
		cat < "$output"
		fail_ "$error"
	}
}

check_prog cat
check_prog rm

LOG="$ME_.tmp"
rm -f "$LOG"

: "${STRACE:=../strace}"
: "${TIMEOUT_DURATION:=60}"
: "${SLEEP_A_BIT:=sleep 1}"
