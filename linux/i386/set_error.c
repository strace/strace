/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	i386_regs.eax = -tcp->u_error;
	return upoke(tcp, 4 * EAX, i386_regs.eax);
}

static int
arch_set_success(struct tcb *tcp)
{
	i386_regs.eax = tcp->u_rval;
	return upoke(tcp, 4 * EAX, i386_regs.eax);
}
