/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

kernel_ulong_t
shuffle_scno(kernel_ulong_t scno)
{
	if (scno < ARM_FIRST_SHUFFLED_SYSCALL)
		return scno;

	/* __ARM_NR_cmpxchg? Swap with LAST_ORDINARY+1 */
	if (scno == ARM_FIRST_SHUFFLED_SYSCALL)
		return 0x000ffff0;
	if (scno == 0x000ffff0)
		return ARM_FIRST_SHUFFLED_SYSCALL;

#define ARM_SECOND_SHUFFLED_SYSCALL (ARM_FIRST_SHUFFLED_SYSCALL + 1)
	/*
	 * Is it ARM specific syscall?
	 * Swap [0x000f0000, 0x000f0000 + LAST_SPECIAL] range
	 * with [SECOND_SHUFFLED, SECOND_SHUFFLED + LAST_SPECIAL] range.
	 */
	if (scno >= 0x000f0000 &&
	    scno <= 0x000f0000 + ARM_LAST_SPECIAL_SYSCALL) {
		return scno - 0x000f0000 + ARM_SECOND_SHUFFLED_SYSCALL;
	}
	if (scno <= ARM_SECOND_SHUFFLED_SYSCALL + ARM_LAST_SPECIAL_SYSCALL) {
		return scno + 0x000f0000 - ARM_SECOND_SHUFFLED_SYSCALL;
	}

	return scno;
}
