/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	kernel_ulong_t scno = 0;

	if (upeek(tcp, REG_A3, &alpha_a3) < 0)
		return -1;
	if (upeek(tcp, REG_R0, &scno) < 0)
		return -1;

	/*
	 * Do some sanity checks to figure out if it's
	 * really a syscall entry
	 */
	if (!scno_in_range(scno)) {
		if (alpha_a3 == 0 || alpha_a3 == -1UL) {
			debug_msg("stray syscall exit: r0 = %lu", scno);
			return 0;
		}
	}

	tcp->scno = scno;
	return 1;
}
