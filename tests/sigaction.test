#!/bin/sh

# Check rt_sigaction decoding.

. "${srcdir=.}/init.sh"

check_prog awk

./sigaction ||
	fail_ 'sigaction failed'

args="-o $LOG -ert_sigaction ./sigaction"
$STRACE $args ||
	fail_ "strace $args failed"

awk -f "$srcdir"/sigaction.awk $LOG ||
	{ cat $LOG; fail_ 'unexpected output'; }

exit 0
