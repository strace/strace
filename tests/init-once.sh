#!/bin/sh
#
# Copyright (c) 2011-2016 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2011-2026 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

export LC_ALL=C
ME_="${0##*/}"
LOG="log"
OUT="out"
EXP="exp"
CONFIG_H="../../src/config.h"

# Starting with glibc 2.43, support for 2MB transparent huge pages
# has been enabled by default in malloc on AArch64.
# Disable it to avoid unexpected madvise() and close() invocations
# that break tests.
GLIBC_TUNABLES=glibc.malloc.hugetlb=0; export GLIBC_TUNABLES

warn_() { printf >&2 '%s\n' "$*"; }
fail_() { warn_ "$ME_: failed test: $*"; exit 1; }
skip_() { warn_ "$ME_: skipped test: $*"; exit 77; }
framework_failure_() { warn_ "$ME_: framework failure: $*"; exit 99; }
framework_skip_() { warn_ "$ME_: framework skip: $*"; exit 77; }

# Enable using string in sed regular expressions by escaping all the special
# characters.
sed_re_escape()
{
	printf "%s" "$*" | sed 's/[].*&^$[\/]/\\&/g'
}

# Enable using string in sed s// command by escaping slash occurrences.
sed_slash_escape()
{
	printf "%s" "$*" | sed 's/[/]/\\&/g'
}

# Usage: get_prefix_value PREFIX_REGEXP
# Print the value from the first ^PREFIX_REGEXP line in the standard input.
get_prefix_value()
{
	local prefix="$1"

	check_prog sed
	sed '/^'"$prefix"'/!d;s///;q'
}

# Calculate an integer approximation of the square root of $1:
# returns 0 and prints the largest number the square of which is no greater
# than $1 if $1 is non-negative, returns 1 and prints nothing otherwise.
sq_root()
{
	# Handling the negative cases
	[ 0 -le "$1" ] || return 1;

	# Handling the 0 and 1 cases
	[ 1 -lt "$1" ] || { echo "$1"; return 0; }

	local upper="$1"
	local lower=1
	local cur

	while :; do
		cur="$(( (lower + upper) / 2 ))"
		[ "$((cur * cur))" -ne "$1" ] || { echo "$cur"; return 0; }
		[ "$((cur * cur))" -lt "$1" ] || { upper="$cur"; continue; }
		[ "$cur" -ne "$lower" ] || { echo "$cur"; return 0; }
		lower="$cur"
	done
}

# get_config_str OPTION
#
# Returns the value of OPTION from config.h (path to which set
# in the CONFIG_H variable).
get_config_str()
{
	sed -E -n 's/#define[[:space:]]*'"$1"'[[:space:]]*"([^"]*)".*/\1/p' \
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
	opt=$(sed -E -n 's/#define[[:space:]]*'"$1"'[[:space:]]*([0-9]+)$/\1/p' \
		"$CONFIG_H")
	if [ -n "$opt" ] && [ "$opt" -ne 0 ]; then
		printf "%s" "$2"
	else
		printf "%s" "${3-}"
	fi
}

# Prints personality designator of the current personality:
# 64, 32, or x32.
print_current_personality_designator()
{
	if [ "x$STRACE_NATIVE_ARCH" = "x$STRACE_ARCH" ]; then
		if [ 'x32' = "$STRACE_NATIVE_ARCH" ]; then
			echo x32
		else
			echo "$((SIZEOF_LONG * 8))"
		fi
	else
		[ 4 -eq "$SIZEOF_LONG" ] ||
			fail_ "sizeof(long) = $SIZEOF_LONG != 4"
		if [ "x$SIZEOF_KERNEL_LONG_T" = "x$SIZEOF_LONG" ]; then
			echo 32
		else
			[ 8 -eq "$SIZEOF_KERNEL_LONG_T" ] ||
				fail_ "sizeof(kernel_long_t) = $SIZEOF_KERNEL_LONG_T != 8"
			echo x32
		fi
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
	"$srcdir"/../src/strace-log-merge "$LOG" > "$LOG" ||
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
		cnt=$((cnt + 1))
	done < "$patterns" ||
		fail_ "Error reading patterns from \"$patterns\""
	test -z "$failed" || {
		echo 'Actual output:'
		cat < "$output"
		fail_ "$error"
	}
}

# Timing comparison helpers (sleep-timing vs strace -T/-r/-cw).
TIMING_FILE="timing"
TIMING_ABS_TOL=0.2
TIMING_REL_TOL=0.05
TIMING_ABS_MIN=0.1

# Usage: timing_quant_slack STRACE_OPT
# Print extra timing slack for STRACE_OPT precision (rounding/truncation).
timing_quant_slack()
{
	case "$1" in
		*=ms|*--syscall-times=ms|*--relative-timestamps=ms)
			echo 0.001 ;;
		*=ns|*--syscall-times=ns|*--relative-timestamps=ns)
			echo 0 ;;
		*) echo 0.01 ;;
	esac
}

# Usage: read_sleep_timing
# Read $TIMING_FILE; print REQUESTED_SEC and NANOSLEEP_SEC on one line.
# Return 1 if the file is missing, incomplete, or the observed sleep
# is shorter than requested by $TIMING_ABS_MIN.
read_sleep_timing()
{
	local requested nanosleep

	[ -f "$TIMING_FILE" ] || {
		warn_ "timing file \"$TIMING_FILE\" not found"
		return 1
	}

	check_prog gawk

	requested=$(get_prefix_value 'requested_sec=' < "$TIMING_FILE")
	nanosleep=$(get_prefix_value 'nanosleep_sec=' < "$TIMING_FILE")

	[ -n "$requested" ] && [ -n "$nanosleep" ] || {
		warn_ "timing file \"$TIMING_FILE\" is incomplete"
		return 1
	}

	gawk -v req="$requested" -v ns="$nanosleep" \
		-v min="$TIMING_ABS_MIN" 'BEGIN {
		if (ns + min < req)
			exit 1
	}' || {
		warn_ "observed sleep shorter than requested" \
			"(requested=$requested nanosleep=$nanosleep)"
		return 1
	}

	printf '%s %s\n' "$requested" "$nanosleep"
}

# Usage: read_sleep_timing_pair VAR_REQUESTED VAR_NANOSLEEP
# Set the named variables to timing file contents; fail the test on error.
read_sleep_timing_pair()
{
	local _timing var_requested var_nanosleep

	[ $# -eq 2 ] ||
		fail_ 'usage: read_sleep_timing_pair' \
			'VAR_REQUESTED VAR_NANOSLEEP'

	var_requested=$1
	var_nanosleep=$2

	_timing=$(read_sleep_timing) ||
		fail_ "failed to read sleep timing from \"$TIMING_FILE\""

	set -- $_timing
	eval "$var_requested=\$1; $var_nanosleep=\$2"
}

# Usage: compare_timing_fp STRACE_VAL PROGRAM_VAL QUANT_SLACK
# Fail unless |STRACE_VAL - PROGRAM_VAL| is within tolerance.
compare_timing_fp()
{
	[ $# -eq 3 ] ||
		fail_ 'usage: compare_timing_fp' \
			'STRACE_VAL PROGRAM_VAL QUANT_SLACK'

	local strace_val="$1"
	local program_val="$2"
	local quant_slack="$3"

	check_prog gawk

	gawk -v s="$strace_val" \
	     -v p="$program_val" \
	     -v abs="$TIMING_ABS_TOL" \
	     -v rel="$TIMING_REL_TOL" \
	     -v quant="$quant_slack" \
		'BEGIN {
		diff = s - p
		if (diff < 0)
			diff = -diff
		tol = abs + 0
		if (rel * p > tol)
			tol = rel * p
		tol += quant + 0
		if (diff > tol) {
			printf "timing mismatch (fp): strace=%g program=%g diff=%g tolerance=%g\n",
				s, p, diff, tol > "/dev/stderr"
			exit 1
		}
	}'
}

# Usage: compare_timing_sec STRACE_VAL PROGRAM_VAL
# Fail unless STRACE_VAL (whole seconds from strace) matches PROGRAM_VAL
# within TIMING_ABS_TOL/TIMING_REL_TOL, allowing truncation of sub-second part.
compare_timing_sec()
{
	[ $# -eq 2 ] ||
		fail_ 'usage: compare_timing_sec STRACE_VAL PROGRAM_VAL'

	local strace_val="$1"
	local program_val="$2"

	check_prog gawk

	gawk -v s="$strace_val" \
	     -v p="$program_val" \
	     -v abs="$TIMING_ABS_TOL" \
	     -v rel="$TIMING_REL_TOL" \
		'BEGIN {
		if (s !~ /^[0-9]+$/) {
			printf "timing mismatch (sec): strace value %s is not an integer\n",
				s > "/dev/stderr"
			exit 1
		}
		s += 0
		lo = int(p - 1e-6)
		hi = int(p + abs + rel * p + 1e-6)
		if (s < lo || s > hi) {
			printf "timing mismatch (sec): strace=%g program=%g allowed [%d,%d]\n",
				s, p, lo, hi > "/dev/stderr"
			exit 1
		}
	}'
}

# Usage: timing_strace_matches_requested REQUESTED_SEC STRACE_VAL STRACE_OPT
# Return 0 if STRACE_VAL matches REQUESTED_SEC within tolerance for STRACE_OPT.
timing_strace_matches_requested()
{
	[ $# -eq 3 ] ||
		fail_ 'usage: timing_strace_matches_requested' \
			'REQUESTED_SEC STRACE_VAL STRACE_OPT'

	local requested_val="$1"
	local strace_val="$2"
	local opt="$3"

	case "$opt" in
		*=s|*--syscall-times=s|*--relative-timestamps=s)
			compare_timing_sec "$strace_val" "$requested_val" ;;
		*)
			compare_timing_fp "$strace_val" "$requested_val" \
				"$(timing_quant_slack "$opt")" ;;
	esac
}

# Usage: timing_scheduling_waiver \
# 	REQUESTED_SEC NANOSLEEP_SEC STRACE_VAL OVERHEAD STRACE_OPT
# Return 0 if NANOSLEEP_SEC exceeds STRACE_VAL because the tracee bracket
# includes post-syscall scheduling delay while strace timed the syscall
# at REQUESTED_SEC.
timing_scheduling_waiver()
{
	[ $# -eq 5 ] ||
		fail_ 'usage: timing_scheduling_waiver' \
			'REQUESTED_SEC NANOSLEEP_SEC STRACE_VAL' \
			'OVERHEAD STRACE_OPT'

	local requested="$1"
	local program_t="$2"
	local strace_t="$3"
	local overhead="$4"
	local opt="$5"
	local requested_adj

	check_prog gawk

	gawk -v s="$strace_t" -v p="$program_t" \
		'BEGIN { if (p <= s) exit 1 }' ||
		return 1

	requested_adj=$(gawk -v r="$requested" -v o="$overhead" \
		'BEGIN { print r - o }')
	timing_strace_matches_requested "$requested_adj" "$strace_t" "$opt"
}

# Usage: timing_compare_cw REQUESTED_SEC NANOSLEEP_SEC STRACE_VAL OVERHEAD
# Compare -c/-cw wall-clock summary time to NANOSLEEP_SEC - OVERHEAD.
timing_compare_cw()
{
	[ $# -eq 4 ] ||
		fail_ 'usage: timing_compare_cw' \
			'REQUESTED_SEC NANOSLEEP_SEC STRACE_VAL OVERHEAD'

	local requested="$1"
	local program_t="$2"
	local strace_t="$3"
	local overhead="$4"
	local expected

	check_prog gawk

	expected=$(gawk -v p="$program_t" -v o="$overhead" \
		'BEGIN { print p - o }')

	compare_timing_fp "$strace_t" "$expected" 0.01 &&
		return 0
	timing_scheduling_waiver "$requested" "$program_t" "$strace_t" \
		"$overhead" '-cw' &&
		return 0
	compare_timing_fp "$strace_t" "$expected" 0.01
}

# Usage: timing_compare_sleep \
# 	REQUESTED_SEC NANOSLEEP_SEC STRACE_VAL OVERHEAD STRACE_OPT
# Compare STRACE_VAL to NANOSLEEP_SEC - OVERHEAD using the check suited
# to STRACE_OPT.
timing_compare_sleep()
{
	[ $# -eq 5 ] ||
		fail_ 'usage: timing_compare_sleep' \
			'REQUESTED_SEC NANOSLEEP_SEC STRACE_VAL' \
			'OVERHEAD STRACE_OPT'

	local requested="$1"
	local program_t="$2"
	local strace_t="$3"
	local overhead="$4"
	local opt="$5"
	local expected
	local quant

	check_prog gawk

	expected=$(gawk -v p="$program_t" -v o="$overhead" \
		'BEGIN { print p - o }')

	case "$opt" in
		*=s|*--syscall-times=s|*--relative-timestamps=s)
			compare_timing_sec "$strace_t" "$expected" &&
				return 0
			timing_scheduling_waiver "$requested" "$program_t" \
				"$strace_t" "$overhead" "$opt" &&
				return 0
			compare_timing_sec "$strace_t" "$expected"
			;;
		*)
			quant=$(timing_quant_slack "$opt")
			compare_timing_fp "$strace_t" "$expected" "$quant" &&
				return 0
			timing_scheduling_waiver "$requested" "$program_t" \
				"$strace_t" "$overhead" "$opt" &&
				return 0
			compare_timing_fp "$strace_t" "$expected" "$quant"
			;;
	esac
}

# Usage: timing_fail \
# 	REQUESTED_SEC NANOSLEEP_SEC STRACE_VAL OVERHEAD OPTION ERROR_MESSAGE
# Print timing diagnostics and fail with ERROR_MESSAGE.
timing_fail()
{
	[ $# -eq 6 ] ||
		fail_ 'usage: timing_fail' \
			'REQUESTED_SEC NANOSLEEP_SEC STRACE_VAL' \
			'OVERHEAD OPTION ERROR_MESSAGE'

	cat >&2 <<EOF
requested_sec=$1 nanosleep_sec=$2
strace=$3 overhead=$4 option=$5
EOF
	dump_log_and_fail_with "$6"
}

# Usage: parse_strace_T_time
# Print the <elapsed> time from the nanosleep line in $LOG.
parse_strace_T_time()
{
	local val

	check_prog sed

	val=$(LC_ALL=C sed -n \
		'/nanosleep({.*) = 0 </{s/.*<\([0-9.]*\)>.*/\1/p;q}' "$LOG")

	[ -n "$val" ] ||
		fail_ "nanosleep -T time not found in \"$LOG\""
	echo "$val"
}

# Usage: parse_strace_r_exit_time
# Print the relative timestamp from the chdir(".") line in $LOG.
parse_strace_r_exit_time()
{
	local val

	check_prog sed

	# s//\1 prints the address's first capture group.
	val=$(LC_ALL=C sed -n \
		'/^[[:space:]]*\([0-9][0-9.]*\)[[:space:]]chdir("\.")[[:space:]]\+= 0$/{
			s//\1/p
			q
		}' "$LOG")

	[ -n "$val" ] ||
		fail_ "relative chdir time not found in \"$LOG\""
	echo "$val"
}

# Usage: check_strace_syscall_time STRACE_OPT
# Compare -T/--syscall-times nanosleep duration in $LOG to sleep-timing output.
check_strace_syscall_time()
{
	[ $# -eq 1 ] ||
		fail_ 'usage: check_strace_syscall_time STRACE_OPT'

	local opt="$1"
	local requested program strace_t

	read_sleep_timing_pair requested program

	strace_t=$(parse_strace_T_time)
	timing_compare_sleep "$requested" "$program" "$strace_t" 0 "$opt" ||
		timing_fail "$requested" "$program" "$strace_t" 0 "$opt" \
			"$STRACE $args syscall time mismatch"
}

# Usage: check_strace_relative_exit_time STRACE_OPT
# Compare -r/--relative-timestamps on the post-sleep chdir line in $LOG
# (delta since the prior traced chdir) to sleep-timing output.
check_strace_relative_exit_time()
{
	[ $# -eq 1 ] ||
		fail_ 'usage: check_strace_relative_exit_time STRACE_OPT'

	local opt="$1"
	local requested program strace_t

	read_sleep_timing_pair requested program

	strace_t=$(parse_strace_r_exit_time)
	timing_compare_sleep "$requested" "$program" "$strace_t" 0 "$opt" ||
		timing_fail "$requested" "$program" "$strace_t" 0 "$opt" \
			"$STRACE $args relative exit time mismatch"
}

# Usage: filter_vdso_calls test_program actual_file [expected_file]
# There are spurious calls to clock_gettime64
# from a combination of glibc 2.42+ needing
# randomness on malloc initialization and
# the platform lacking vDSO.
# Since those calls cannot be distinguished
# from expected calls, sanitize the whole lot.
filter_vdso_calls()
{
	if [ "$SIZEOF_LONG" -eq 4 ]; then
		[ -f "$1" ] || fail_ "test program '$1' non existent"
		ldd "$1" | grep -q 'linux-vdso.so.1' || {
			sed_expr='/^clock_gettime64(CLOCK_MONOTONIC,.*= 0$/d'
			sed -i "$sed_expr" "$2"
			if [ -f "${3-}" ]; then
				sed -i "$sed_expr" "$3"
			fi
		}
	fi
}

# Usage: run_strace_match_diff [args to run_strace]
run_strace_match_diff()
{
	local sed_cmd prog_args
	prog_args="../$NAME"
	sed_cmd='p'

	args="$*"
	[ -n "$args" ] && [ -z "${args##*-e trace=*}" -o \
			    -z "${args##*-etrace=*}" -o \
			    -z "${args##*--trace=*}" ] ||
		set -- -e trace="$NAME" "$@"

	set -- "$@" END_OF_ARGUMENTS
	while :; do
		arg="$1"
		shift
		case "$arg" in
		QUIRK:START-OF-TEST-OUTPUT:*)
			str="${arg#QUIRK:START-OF-TEST-OUTPUT:}"
			sed_cmd="/$(sed_re_escape "$str")/,\$p"
			continue
			;;
		QUIRK:START-OF-TEST-OUTPUT-REGEX:*)
			str="${arg#QUIRK:START-OF-TEST-OUTPUT-REGEX:}"
			sed_cmd="/$(sed_slash_escape "$str")/,\$p"
			continue
			;;
		QUIRK:PROG-ARGS:*)
			prog_args="../$NAME ${arg#QUIRK:PROG-ARGS:}"
			continue
			;;
		END_OF_ARGUMENTS)
			break
			;;
		esac

		set -- "$@" "$arg"
	done

	run_prog $prog_args > /dev/null
	run_strace "$@" $args > "$EXP"
	sed -n "$sed_cmd" < "$LOG" > "$OUT"
	filter_vdso_calls "../$NAME" "$OUT" "$EXP"
	match_diff "$OUT" "$EXP"
}

# Usage: run_strace_match_grep [args to run_strace]
run_strace_match_grep()
{
	args="$*"
	[ -n "$args" ] && [ -z "${args##*-e trace=*}" -o \
			    -z "${args##*-etrace=*}" -o \
			    -z "${args##*--trace=*}" ] ||
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
		echo "$((v1 * 65536 + v2 * 256 + v3))"
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

# Usage: require_max_kernel_version_or_skip 6.11
require_max_kernel_version_or_skip()
{
	local uname_r
	uname_r="$(uname -r)"

	[ "$(kernel_version_code "$uname_r")" -lt \
	  "$(kernel_version_code "$1")" ] ||
		skip_ "the kernel release $uname_r is $1 or newer"
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
	local expfile sed_expr

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
		filter_vdso_calls "../$t" "$LOG" "$expfile"

		case "$STRACE_ARCH:$MIPS_ABI:$NAME" in
			mips:o32:*creds)
				sed -i '/^prctl(PR_GET_FP_MODE)  *= 0$/d' "$LOG"
				;;
		esac

		match_diff "$LOG" "$expfile"
	} < /dev/null; done
}

# Run strace against list of programs put in "$NAME.in" and then against
# a random selection of the binaries from the rest of pure_executables.list
# with the expectation of empty output in the latter case.
#
# Usage: source this file after init.sh and call:
#   test_trace_expr subtrahend_regexp strace_args
# Environment:
#   $NAME:	test name, used for "$NAME.in" file containing list of tests
#		for positive trace expression match;
#   $srcdir:	used to find pure_executables.list and "$NAME.in" files.
#   $TRACE_TESTS_SAMPLE:
#               number of binaries to pick from the remainder
#               of pure_executables.list to check for the absence of output.
# Files created:
#   negative.list: File containing list of tests for negative match.
test_trace_expr()
{
	local subtrahend_regexp
	subtrahend_regexp="$1"; shift
	test_pure_prog_set "$@" < "$srcdir/$NAME.in"
	prog_set_subtract "$srcdir/pure_executables.list" "$srcdir/$NAME.in" \
		"$subtrahend_regexp" | shuf -n "${TRACE_TESTS_SAMPLE}"  > negative.list
	test_pure_prog_set --expfile /dev/null -qq -esignal=none "$@" \
		< negative.list
}

test_prog_set()
{
	test_pure_prog_set "$@" < "$srcdir/$NAME.in"
}

test_pidns_run_strace()
{
	local parent_pid init_pid sed_cmd prog_args
	prog_args="../$NAME"
	sed_cmd='p'

	check_prog tail
	check_prog cut
	check_prog grep

	set -- "$@" END_OF_ARGUMENTS
	while :; do
		arg="$1"
		shift
		case "$arg" in
		QUIRK:START-OF-TEST-OUTPUT:*)
			str="${arg#QUIRK:START-OF-TEST-OUTPUT:}"
			str="$(sed_re_escape "${str}")"
			# There could be -r/-t output between pid and "+++"
			sed_cmd="/${str}/,/^[1-9][0-9]* .*+++ exited with 0 +++\$/p"
			continue
			;;
		QUIRK:START-OF-TEST-OUTPUT-REGEX:*)
			str="${arg#QUIRK:START-OF-TEST-OUTPUT-REGEX:}"
			str="$(sed_slash_escape "${str}")"
			# There could be -r/-t output between pid and "+++"
			sed_cmd="/${str}/,/^[1-9][0-9]* .*+++ exited with 0 +++\$/p"
			continue
			;;
		QUIRK:PROG-ARGS:*)
			prog_args="../$NAME ${arg#QUIRK:PROG-ARGS:}"
			continue
			;;
		END_OF_ARGUMENTS)
			break
			;;
		esac

		set -- "$@" "$arg"
	done

	run_prog > /dev/null
	args="$prog_args"
	run_strace --decode-pids=pidns --status=!unavailable -f "$@" $args > "$EXP"

	# filter out logs made by the parent or init process of the pidns test
	parent_pid="$(tail -n 2 $LOG | head -n 1 | cut -d' ' -f1)"
	init_pid="$(tail -n 1 $LOG | cut -d' ' -f1)"
	grep -E -v "^($parent_pid|$init_pid) " "$LOG" | sed -n "$sed_cmd" > "$OUT"
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

check_scno_tampering()
{
	case "$STRACE_ARCH" in
		arm)
			# PTRACE_SET_SYSCALL is supported by linux kernel
			# starting with commit v2.6.16-rc1~107^2.
			require_min_kernel_version_or_skip 2.6.16 ;;
		aarch64)
			# NT_ARM_SYSTEM_CALL regset is supported by linux kernel
			# starting with commit v3.19-rc1~59^2~16.
			require_min_kernel_version_or_skip 3.19 ;;
		hppa)
			# Syscall number and return value modification did not work
			# properly before commit v4.5-rc7~31^2~1.
			require_min_kernel_version_or_skip 4.5 ;;
		sparc*)
			# Reloading the syscall number from %g1 register is supported
			# by linux kernel starting with commit v4.5-rc7~35^2~3.
			require_min_kernel_version_or_skip 4.5 ;;
		mips)
			# Only the native ABI is supported by the kernel properly, see
			# https://lists.strace.io/pipermail/strace-devel/2017-January/005896.html
			msg_prefix="mips $MIPS_ABI scno tampering does not work"
			uname_m="$(uname -m)"
			case "$MIPS_ABI:$uname_m" in
				n64:mips64) ;;
				o32:mips)
					# is it really mips32?
					if ../is_linux_mips_n64; then
						skip_ "$msg_prefix on mips n64 yet"
					fi
					;;
				*) skip_ "$msg_prefix on $uname_m yet" ;;
			esac ;;
	esac
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
	rm -rf -- "$TESTDIR" &&
	mkdir -- "$TESTDIR" &&
	cd "$TESTDIR" ||
	framework_failure_ "Cannot setup $TESTDIR"

	case "$srcdir" in
		/*) ;;
		*) srcdir="../$srcdir" ;;
	esac

	[ -n "${STRACE-}" ] || {
		STRACE=../../src/strace
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
	: "${STRACE:=../src/strace}"
fi

# Export $STRACE_EXE to check_PROGRAMS.
: "${STRACE_EXE:=$STRACE}"
export STRACE_EXE

: "${TIMEOUT_DURATION:=1500}"
: "${SLEEP_A_BIT:=sleep 1}"
: "${TRACE_TESTS_SAMPLE:=100}"

[ -z "${VERBOSE-}" ] ||
	set -x
