/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	for (unsigned int i = 0; i < MAX_ARGS; ++i) {
		/* arguments go backwards from D1Ar1 (D1.3) */
		tcp->u_arg[i] = (&metag_regs.dx[3][1])[-i];
	}
	return 1;
}
