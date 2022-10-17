#!/bin/sh
#
# Check prctl syscall decoding.
#
# Copyright (c) 2021-2022 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/init.sh"

check_prog sed
run_prog > /dev/null
run_strace -eprctl "$@" $args > "$EXP"
sed '0,/^prctl(0xffffffff\( \/\* PR_??? \*\/\)\?, 0xfffffffe, 0xfffffffd, 0xfffffffc, 0xfffffffb)  *= -1 /d' < "$LOG" > "$OUT"
match_diff "$OUT" "$EXP"
