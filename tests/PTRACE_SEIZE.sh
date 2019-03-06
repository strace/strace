#!/bin/sh
#
# Skip the test if PTRACE_SEIZE is not supported.
#
# Copyright (c) 2014-2019 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

$STRACE -d -enone / > /dev/null 2> "$LOG" ||:
if grep -x "[^:]*strace: PTRACE_SEIZE doesn't work" "$LOG" > /dev/null; then
	skip_ "PTRACE_SEIZE doesn't work"
fi
