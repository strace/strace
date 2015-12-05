#!/bin/sh

# Check decoding of stat family syscalls.

. "${srcdir=.}/init.sh"

syscall=${ME_%.test}
run_prog > /dev/null
OUT="$LOG.out"
run_strace -ve$syscall -P$syscall.sample $args > "$OUT"
match_diff "$LOG" "$OUT"
rm -f "$OUT"

exit 0
