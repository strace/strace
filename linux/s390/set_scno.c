/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef ARCH_REGSET
# define ARCH_REGSET s390_regset
#endif

static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	ARCH_REGSET.gprs[2] = scno;
	return set_regs(tcp->pid);
}
