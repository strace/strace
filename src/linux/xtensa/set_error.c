/*
 * Copyright (c) 2016-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	xtensa_regs.a[xtensa_regs.windowbase * 4 + 2] = -tcp->u_error;
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	xtensa_regs.a[xtensa_regs.windowbase * 4 + 2] = tcp->u_rval;
	return set_regs(tcp->pid);
}
