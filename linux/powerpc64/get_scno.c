/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	tcp->scno = ppc_regs.gpr[0];
	/*
	 * Check for 64/32 bit mode.
	 * Embedded implementations covered by Book E extension of PPC use
	 * bit 0 (CM) of 32-bit Machine state register (MSR).
	 * Other implementations use bit 0 (SF) of 64-bit MSR.
	 */
	unsigned int currpers = (ppc_regs.msr & 0x8000000080000000) ? 0 : 1;
	update_personality(tcp, currpers);
	return 1;
}
