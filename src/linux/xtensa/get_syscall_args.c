/*
 * Copyright (c) 2015-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static unsigned int xtensa_probe_naregs(struct tcb *tcp)
{
	struct user_pt_regs tmp = xtensa_regs;
	unsigned int n = 8;

	do {
		n *= 2;
		xtensa_regs.windowbase = n / 4;
	} while (n < ARRAY_SIZE(xtensa_regs.a) &&
		 set_regs(tcp->pid) == 0);
	xtensa_regs = tmp;
	set_regs(tcp->pid);
	return n;
}

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	static const unsigned int syscall_regs[MAX_ARGS] = { 6, 3, 4, 5, 8, 9 };
	static unsigned int naregs_mask;

	if (naregs_mask == 0)
		naregs_mask = xtensa_probe_naregs(tcp) - 1;

	for (unsigned int i = 0; i < n_args(tcp); ++i)
		tcp->u_arg[i] = xtensa_regs.a[(xtensa_regs.windowbase * 4 +
					       syscall_regs[i]) & naregs_mask];
	return 1;
}
