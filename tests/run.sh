#!/bin/sh

. "${srcdir=.}/init.sh"

$STRACE -V > /dev/null ||
	framework_failure_ "$STRACE is not available"

TIMEOUT="timeout -k 5 -s XCPU $TIMEOUT_DURATION"
$TIMEOUT true > /dev/null 2>&1 ||
	TIMEOUT=

exec $TIMEOUT "$@"
