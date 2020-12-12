/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "semtimedop-common.c"

static long
k_semtimedop_imp(const kernel_ulong_t semid,
		 const kernel_ulong_t sops,
		 const kernel_ulong_t nsops,
		 const kernel_ulong_t timeout)
{
	static const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	return syscall(SYSCALL_NR, semid, sops, nsops, timeout, bad, bad);
}
