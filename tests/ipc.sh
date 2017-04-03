#!/bin/sh

# Check decoding of ipc syscalls

. "${srcdir=.}/init.sh"

run_prog > /dev/null
run_strace -eipc $args > "$EXP"
match_grep "$LOG" "$EXP"

exit 0
