/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef ARCH_REGSET
# define ARCH_REGSET s390_regset
#endif

static int
arch_set_error(struct tcb *tcp)
{
	ARCH_REGSET.gprs[2] = -tcp->u_error;
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	ARCH_REGSET.gprs[2] = tcp->u_rval;
	return set_regs(tcp->pid);
}
