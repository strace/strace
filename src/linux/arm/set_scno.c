/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PTRACE_SET_SYSCALL
# define PTRACE_SET_SYSCALL 23
#endif
/*
 * PTRACE_SET_SYSCALL is supported by linux kernel
 * starting with commit v2.6.16-rc1~107^2
 */

static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	unsigned int n = (uint16_t) scno;
	int rc = ptrace(PTRACE_SET_SYSCALL, tcp->pid, NULL, (unsigned long) n);
	if (rc && errno != ESRCH)
		perror_msg("arch_set_scno: PTRACE_SET_SYSCALL pid:%d scno:%#x",
			   tcp->pid, n);
	return rc;
}
