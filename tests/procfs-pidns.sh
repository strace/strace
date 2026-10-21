#!/bin/sh
#
# Run a generated test with procfs in an ancestor PID namespace.
#
# Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/init.sh"
check_prog unshare
unshare -Urpf -- true > /dev/null 2>&1 ||
	framework_skip_ 'unshare -Urpf -- true failed'

NAME="${NAME%--pidns}"
STRACE="unshare -Urpf -- $STRACE"
. "${srcdir}/$NAME.gen.test"
