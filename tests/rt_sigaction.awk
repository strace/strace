#!/bin/gawk
#
# Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2016 Elvira Khabirova <lineprinter0@gmail.com>
# Copyright (c) 2016-2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

BEGIN {
	n1[1][1] = n2[1][1] = "SIG_IGN"
	n1[1][2] = n2[1][2] = "\\[HUP INT\\]"
	n1[1][3] = "SA_RESTORER\\|SA_RESTART"
	n2[1][3] = "SA_RESTART"
	n1[1][4] = ", sa_restorer=0x[0-9a-f]+"
	n2[1][4] = ""

	n1[2][1] = n2[2][1] = "0x[0-9a-f]+"
	n1[2][2] = n2[2][2] = "\\[QUIT TERM\\]"
	n1[2][3] = "SA_RESTORER\\|SA_SIGINFO"
	n2[2][3] = "SA_SIGINFO"
	n1[2][4] = ", sa_restorer=0x[0-9a-f]+"
	n2[2][4] = ""

	n1[3][1] = n2[3][1] = "SIG_DFL"
	n1[3][2] = n2[3][2] = "\\[\\]"
	n1[3][3] = "SA_RESTORER"
	n2[3][3] = "0"
	n1[3][4] = ", sa_restorer=0x[0-9a-f]+"
	n2[3][4] = ""

	n1[4][1] = n2[4][1] = "SIG_DFL"
	n1[4][2] = n2[4][2] = "~\\[HUP( ((RT|SIGRT)[^] ]+|[3-9][0-9]|1[0-9][0-9]))*\\]"
	n1[4][3] = "SA_RESTORER"
	n2[4][3] = "0"
	n1[4][4] = ", sa_restorer=0x[0-9a-f]+"
	n2[4][4] = ""

	o1[1][1] = o2[1][1] = "SIG_DFL"
	o1[1][2] = o2[1][2] = "\\[\\]"
	o1[1][3] = o2[1][3] = "0"
	o1[1][4] = o2[1][4] = ""

	for (i = 2; i < 5; i++) {
		for (j = 1; j < 5; j++) {
			o1[i][j] = n1[i - 1][j]
			o2[i][j] = n2[i - 1][j]
		}
	}

	a1 = "(0x[0-9a-f]+, )?(4|8|16)"
	a2 = "(4|8|16)(, 0x[0-9a-f]+)?"
	a3 = "0x[0-9a-f]+, (4|8|16)"

	for (i = 1; i < 5; i++) {
		r[i] = "^rt_sigaction\\(SIGUSR2, (" \
			"\\{sa_handler=" n1[i][1] ", sa_mask=" n1[i][2] \
			", sa_flags=" n1[i][3] n1[i][4] "\\}, \\{sa_handler=" \
			o1[i][1] ", sa_mask=" o1[i][2] ", sa_flags=" o1[i][3] \
			o1[i][4] "\\}, " a1 "|" \
			"\\{sa_handler=" n2[i][1] ", sa_mask=" n2[i][2] \
			", sa_flags=" n2[i][3] n2[i][4] "\\}, \\{sa_handler=" \
			o2[i][1] ", sa_mask=" o2[i][2] ", sa_flags=" o2[i][3] \
			o2[i][4] "\\}, " a2 "|" \
			"\\{sa_handler=" n2[i][1] ", sa_mask=" n2[i][2] \
			", sa_flags=" n2[i][3] n2[i][4] "\\}, \\{sa_handler=" \
			o2[i][1] ", sa_mask=" o2[i][2] ", sa_flags=" o2[i][3] \
			o2[i][4] "\\}, " a3 ")\\) = 0$"
	}
	s[5] = "+++ exited with 0 +++"

	lines = 5
	fail = 0
}

@include "match.awk"
