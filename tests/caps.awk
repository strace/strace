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
	cap = "(0|CAP_[A-Z_]+(\\|CAP_[A-Z_]+)*|CAP_[A-Z_]+(\\|CAP_[A-Z_]+){37}\\|0xffffffc0)"
	r[1] = "^capget\\(\\{_LINUX_CAPABILITY_VERSION_3, 0\\}, \\{" cap ", " cap ", " cap "\\}\\) = 0$"
	capset_data = "{CAP_DAC_OVERRIDE|CAP_WAKE_ALARM, CAP_DAC_READ_SEARCH|CAP_BLOCK_SUSPEND, 0}"
	s[2] = "capset({_LINUX_CAPABILITY_VERSION_3, 0}, " capset_data ") = -1 EPERM (Operation not permitted)"
	s[3] = "+++ exited with 0 +++"

	lines = 3
	fail = 0
}

@include "match.awk"
