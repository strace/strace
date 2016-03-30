#!/bin/sh

# Check decoding of ipc syscalls

. "${srcdir=.}/init.sh"

run_prog > /dev/null
run_strace -eipc $args > "$OUT"
match_grep "$LOG" "$OUT"

rm -f "$OUT"

exit 0
