/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef ARCH_REGSET
# define ARCH_REGSET s390_regset
#endif

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	tcp->u_arg[0] = ARCH_REGSET.orig_gpr2;
	tcp->u_arg[1] = ARCH_REGSET.gprs[3];
	tcp->u_arg[2] = ARCH_REGSET.gprs[4];
	tcp->u_arg[3] = ARCH_REGSET.gprs[5];
	tcp->u_arg[4] = ARCH_REGSET.gprs[6];
	tcp->u_arg[5] = ARCH_REGSET.gprs[7];
	return 1;
}
