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
	tcp->scno = mips_REG_V0;

	if (!scno_in_range(tcp->scno)) {
		if (mips_REG_A3 == 0 || mips_REG_A3 == (uint64_t) -1) {
			debug_msg("stray syscall exit: v0 = %ld", tcp->scno);
			return 0;
		}
	}

	return 1;
}
