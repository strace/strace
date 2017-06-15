#!/bin/sh
#
# Copyright (c) 2011-2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

ME_="${0##*/}"
LOG="log"
OUT="out"
EXP="exp"
ASINFO="../../asinfo"

fail_() { warn_ "$ME_: failed test: $*"; exit 1; }
warn_() { printf >&2 '%s\n' "$*"; }

run_prog()
{
	if [ $# -eq 0 ]; then
		set -- "../$NAME"
	fi
	args="$*"
	"$@" || {
		rc=$?
		if [ $rc != 0 ]; then
			fail_ "$args failed with code $rc"
		fi
	}
}


dump_log_and_fail_with()
{
	cat < "$LOG" >&2
	fail_ "$*"
}

run_asinfo()
{
	args="$*"
	$ASINFO "$@" 2>&1
}

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

	diff -u -- "$expected" "$output" ||
		fail_ "$error"
}

NAME="${ME_%.test}"
TESTDIR="$NAME.dir"
rm -rf -- "$TESTDIR"
mkdir -- "$TESTDIR"
cd "$TESTDIR"
case "$srcdir" in
	/*) ;;
	*) srcdir="../$srcdir" ;;
esac
