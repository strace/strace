#!/bin/gawk
#
# Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
# Copyright (c) 2016 Elvira Khabirova <lineprinter0@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
