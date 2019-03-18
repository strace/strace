/*
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	/* arg0: a6, arg1: a3, arg2: a4, arg3: a5, arg4: a8, arg5: a9 */
	static const int xtensaregs[MAX_ARGS] = {
		REG_A_BASE + 6,
		REG_A_BASE + 3,
		REG_A_BASE + 4,
		REG_A_BASE + 5,
		REG_A_BASE + 8,
		REG_A_BASE + 9
	};
	unsigned int i;

	for (i = 0; i < n_args(tcp); ++i)
		if (upeek(tcp, xtensaregs[i], &tcp->u_arg[i]) < 0)
			return -1;
	return 1;
}
