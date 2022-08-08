#!/bin/sh
#
# Check arch_prctl syscall decoding.
#
# Copyright (c) 20212-2022 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/init.sh"

check_prog sed
run_prog > /dev/null
run_strace -earch_prctl "$@" $args > "$EXP"
sed '0,/^arch_prctl(0xffffffff\( \/\* ARCH_??? \*\/\)\?, 0xfffffffe)  *= -1 /d' < "$LOG" > "$OUT"
match_diff "$OUT" "$EXP"
