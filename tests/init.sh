#!/bin/sh

ME_="${0##*/}"

LOG="$ME_.tmp"
rm -f "$LOG"

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

: "${STRACE:=../strace}"
: "${TIMEOUT_DURATION:=60}"
: "${SLEEP_A_BIT:=sleep 1}"
