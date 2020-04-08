#!/bin/sh
#
# Skip the test if arch+kernel combination is not supported.
#
# Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
# Copyright (c) 2016-2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

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
		# https://lists.strace.io/pipermail/strace-devel/2017-January/005896.html
		msg_prefix="mips $MIPS_ABI scno tampering does not work"
		uname_m="$(uname -m)"
		case "$MIPS_ABI:$uname_m" in
			n64:mips64) ;;
			o32:mips)
				# is it really mips32?
				if ../is_linux_mips_n64; then
					skip_ "$msg_prefix on mips n64 yet"
				fi
				;;
			*) skip_ "$msg_prefix on $uname_m yet" ;;
		esac ;;
esac
