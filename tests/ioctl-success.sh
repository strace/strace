#!/bin/sh -efu
#
# Check decoding of ioctls using syscall injection.
#
# Expects a binary that accepts IOCTL_INJECT_START as the first argument.
#
# Copyright (c) 2018-2020 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/scno_tampering.sh"

: ${IOCTL_INJECT_START=256}
: ${IOCTL_INJECT_RETVAL=42}

run_strace -a50 "$@" -e trace=ioctl \
	-e inject=ioctl:retval="${IOCTL_INJECT_RETVAL}":when="${IOCTL_INJECT_START}+" \
	"../$NAME" "${IOCTL_INJECT_START}" "${IOCTL_INJECT_RETVAL}" > "$EXP"
grep -v '^ioctl([012][,<]' < "$LOG" > "$OUT"
match_diff "$OUT" "$EXP"
