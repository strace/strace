/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	tile_regs.regs[0] = -tcp->u_error;
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	tile_regs.regs[0] = tcp->u_rval;
	return set_regs(tcp->pid);
}
