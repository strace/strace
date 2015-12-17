#!/bin/gawk
#
# Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
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
	n1[1] = "SIG_IGN, \\[HUP INT\\], SA_RESTORER\\|SA_RESTART, 0x[0-9a-f]+"
	n2[1] = "SIG_IGN, \\[HUP INT\\], SA_RESTART"

	n1[2] = "0x[0-9a-f]+, \\[QUIT TERM\\], SA_RESTORER\\|SA_SIGINFO, 0x[0-9a-f]+"
	n2[2] = "0x[0-9a-f]+, \\[QUIT TERM\\], SA_SIGINFO"

	n1[3] = "SIG_DFL, \\[\\], SA_RESTORER, 0x[0-9a-f]+"
	n2[3] = "SIG_DFL, \\[\\], 0"

	n1[4] = "SIG_DFL, ~\\[HUP( ((RT|SIGRT)[^] ]+|[3-9][0-9]|1[0-9][0-9]))*\\], SA_RESTORER, 0x[0-9a-f]+"
	n2[4] = "SIG_DFL, ~\\[HUP( ((RT|SIGRT)[^] ]+|[3-9][0-9]|1[0-9][0-9]))*\\], 0"

	o1[1] = o2[1] = "SIG_DFL, \\[\\], 0"

	for (i = 2; i < 5; i++) {
		o1[i] = n1[i - 1]
		o2[i] = n2[i - 1]
	}

	a1 = "(0x[0-9a-f]+, )?(4|8|16)"
	a2 = "(4|8|16)(, 0x[0-9a-f]+)?"
	a3 = "0x[0-9a-f]+, (4|8|16)"

	for (i = 1; i < 5; i++) {
		r[i] = "^rt_sigaction\\(SIGUSR2, (" \
			"\\{" n1[i] "\\}, \\{" o1[i] "\\}, " a1 "|" \
			"\\{" n2[i] "\\}, \\{" o2[i] "\\}, " a2 "|" \
			"\\{" n2[i] "\\}, \\{" o2[i] "\\}, " a3 ")\\) = 0$"
	}
	s[5] = "+++ exited with 0 +++"

	lines = 5
	fail = 0
}

@include "match.awk"
