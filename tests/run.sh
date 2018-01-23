#!/bin/sh

. "${srcdir=.}/init.sh"

$STRACE -V > /dev/null ||
	framework_failure_ "$STRACE is not available"

TIMEOUT="timeout -k 5 -s XCPU $TIMEOUT_DURATION"
$TIMEOUT true > /dev/null 2>&1 ||
	TIMEOUT=

if [ $# -eq 0 ]; then
	echo 'No command or test-file specified' >&2
	exit 1
fi

exec $TIMEOUT "$@" < /dev/null
