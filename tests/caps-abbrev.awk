#!/bin/gawk
#
# This file is part of caps strace test.
#
# Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
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
	cap = "(0|1<<CAP_[A-Z_]+(\\|1<<CAP_[A-Z_]+)*|1<<CAP_[A-Z_]+(\\|1<<CAP_[A-Z_]+){37}\\|0xffffffc0)"
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
