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
#if defined(__CSKYABIV2__)
	tcp->scno = csky_regs.regs[3];
#else
	tcp->scno = csky_regs.a1;
#endif
	return 1;
}
