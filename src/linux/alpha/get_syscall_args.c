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
	for (unsigned int i = 0; i < n_args(tcp); ++i)
		if (upeek(tcp, REG_A0+i, &tcp->u_arg[i]) < 0)
			return -1;
	return 1;
}
