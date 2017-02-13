#!/bin/sh
#
# Skip the test if arch+kernel combination is not supported.
#
# Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

. "${srcdir=.}/init.sh"

uname_r="$(uname -r)"
case "$STRACE_ARCH" in
	arm)
		# PTRACE_SET_SYSCALL is supported by linux kernel
		# starting with commit v2.6.16-rc1~107^2.
		require_min_kernel_version_or_skip 2.6.16 ;;
	aarch64)
		# NT_ARM_SYSTEM_CALL regset is supported by linux kernel
		# starting with commit v3.19-rc1~59^2~16.
		require_min_kernel_version_or_skip 3.19 ;;
	hppa)
		# Syscall number and return value modification did not work
		# properly before commit v4.5-rc7~31^2~1.
		require_min_kernel_version_or_skip 4.5 ;;
	sparc*)
		# Reloading the syscall number from %g1 register is supported
		# by linux kernel starting with commit v4.5-rc7~35^2~3.
		require_min_kernel_version_or_skip 4.5 ;;
	mips)
		# Only the native ABI is supported by the kernel properly, see
		# https://sourceforge.net/p/strace/mailman/message/35587571/
		uname_m="$(uname -m)"
		case "$MIPS_ABI:$uname_m" in
			o32:mips|n64:mips64) ;;
			*) skip_ "$MIPS_ABI scno tampering does not work on $uname_m yet" ;;
		esac ;;
esac
