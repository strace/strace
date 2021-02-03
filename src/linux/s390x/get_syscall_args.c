/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define arch_get_syscall_args s390_get_syscall_args
#define ARCH_REGSET s390_regset
#include "../s390/get_syscall_args.c"
#undef ARCH_REGSET
#undef arch_get_syscall_args

#define arch_get_syscall_args s390x_get_syscall_args
#define ARCH_REGSET s390x_regset
#include "../s390/get_syscall_args.c"
#undef ARCH_REGSET
#undef arch_get_syscall_args

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	if (tcp->currpers == 1)
		return s390_get_syscall_args(tcp);
	else
		return s390x_get_syscall_args(tcp);
}
