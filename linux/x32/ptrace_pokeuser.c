/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static long
ptrace_pokeuser(int pid, unsigned long off, kernel_ulong_t val)
{
	/*
	 * As PTRACE_POKEUSER is crippled on x32 by design from the very first
	 * linux kernel commit v3.4-rc1~33^2~2 when it was introduced,
	 * workaround this by using the raw x86_64 syscall instead.
	 */
	return syscall(101, PTRACE_POKEUSER, pid, off, val);
}
