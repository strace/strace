#!/bin/sh

# Check non-abbreviated decoding of ioctls.

. "${srcdir=.}/init.sh"

check_prog grep
run_prog > /dev/null
run_strace -a16 -v -eioctl $args > "$EXP"
grep -v '^ioctl([012],' < "$LOG" > "$OUT"
match_diff "$OUT" "$EXP"
