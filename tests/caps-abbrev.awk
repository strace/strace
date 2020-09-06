#!/bin/gawk
#
# This file is part of caps strace test.
#
# Copyright (c) 2014-2018 Dmitry V. Levin <ldv@altlinux.org>
# Copyright (c) 2016-2020 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

BEGIN {
	cap = "(0|1<<CAP_[A-Z_]+(\\|1<<CAP_[A-Z_]+)*|1<<CAP_[A-Z_]+(\\|1<<CAP_[A-Z_]+){40}\\|0xfffffe00)"
	s_efault = "-1 EFAULT (Bad address)"
	r_efault = "-1 EFAULT \\(Bad address\\)"
	r_einval = "-1 EINVAL \\(Invalid argument\\)"
	r_eperm = "-1 EPERM \\(Operation not permitted\\)"
	r_addr = "0x[[:xdigit:]]+"
	s[1] = "capget(NULL, NULL) = " s_efault
	r[2] = r[3] = "^capget\\(" r_addr ", " r_addr "\\) = " r_efault
	r[4] = "^capget\\(" r_addr ", " r_addr "\\) = 0"

	s[5] = "capset(NULL, NULL) = " s_efault
	r[6] = "^capset\\(" r_addr ", " r_addr "\\) = " r_efault
	r[7] = "^capset\\(" r_addr ", " r_addr "\\) = " r_einval
	r[8] = "^capset\\(" r_addr ", " r_addr "\\) = " r_efault
	r[9] = r[10] = r[11] = "^capset\\(" r_addr ", " r_addr "\\) = " r_eperm
	s[12] = "+++ exited with 0 +++"

	lines = 12
	fail = 0
}

@include "match.awk"
