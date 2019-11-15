#!/bin/sh
#
# Skip the test if seccomp filter is not available.
#
# Copyright (c) 2018-2019 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

$STRACE --seccomp-bpf -f -e trace=fchdir / > /dev/null 2> "$LOG" ||:
if grep -x "[^:]*strace: seccomp filter is requested but unavailable" \
   "$LOG" > /dev/null; then
	skip_ 'seccomp filter is unavailable'
fi
