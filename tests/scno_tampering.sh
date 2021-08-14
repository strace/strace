#!/bin/sh
#
# Skip the test if arch+kernel combination is not supported.
#
# Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2016-2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/init.sh"

check_scno_tampering
