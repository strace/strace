/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

kernel_ulong_t
shuffle_scno_pers(kernel_ulong_t scno, int pers)
{
	if (pers == 0 && scno != (kernel_ulong_t) -1)
		scno ^= __X32_SYSCALL_BIT;

	return scno;
}
