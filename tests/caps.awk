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
	r_addr = "0x[[:xdigit:]]+"
	s[1] = "capget(NULL, NULL) = " s_efault
	r[2] = "^capget\\(" r_addr ", " r_addr "\\) = " r_efault
	r[3] = "^capget\\(\\{version=_LINUX_CAPABILITY_VERSION_3, pid=0\\}, " r_addr "\\) = " r_efault
	r[4] = "^capget\\(\\{version=_LINUX_CAPABILITY_VERSION_3, pid=0\\}, \\{effective=" cap ", permitted=" cap ", inheritable=" cap "\\}\\) = 0$"

	capset_v1_data = "{effective=1<<CAP_DAC_OVERRIDE, permitted=1<<CAP_DAC_READ_SEARCH, inheritable=0}"
	capset_v3hi_data = "{effective=1<<CAP_WAKE_ALARM, permitted=1<<CAP_BLOCK_SUSPEND, inheritable=0}"
	capset_data = "{effective=1<<CAP_DAC_OVERRIDE|1<<CAP_WAKE_ALARM, permitted=1<<CAP_DAC_READ_SEARCH|1<<CAP_BLOCK_SUSPEND, inheritable=0}"
	s[5] = "capset(NULL, NULL) = " s_efault
	r[6] = "^capset\\(" r_addr ", " r_addr "\\) = " r_efault
	r[7] = "^capset\\(\\{version=0xbadc0ded /\\* _LINUX_CAPABILITY_VERSION_\\?\\?\\? \\*/, pid=-1576685468\\}, " r_addr "\\) = " r_einval
	r[8] = "^capset\\(\\{version=_LINUX_CAPABILITY_VERSION_2, pid=0\\}, " r_addr "\\) = " r_efault
	s[9] = "capset({version=_LINUX_CAPABILITY_VERSION_3, pid=0}, " capset_data ") = -1 EPERM (Operation not permitted)"
	s[10] = "capset({version=_LINUX_CAPABILITY_VERSION_3, pid=0}, " capset_v3hi_data ") = -1 EPERM (Operation not permitted)"
	s[11] = "capset({version=_LINUX_CAPABILITY_VERSION_1, pid=0}, " capset_v1_data ") = -1 EPERM (Operation not permitted)"
	s[12] = "+++ exited with 0 +++"

	lines = 12
	fail = 0
}

@include "match.awk"
