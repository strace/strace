/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef NT_ARM_SYSTEM_CALL
# define NT_ARM_SYSTEM_CALL 0x404
#endif
/*
 * NT_ARM_SYSTEM_CALL regset is supported by linux kernel
 * starting with commit v3.19-rc1~59^2~16.
 */

static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	unsigned int n = (uint16_t) scno;
	const struct iovec io = {
		.iov_base = &n,
		.iov_len = sizeof(n)
	};
	int rc = ptrace(PTRACE_SETREGSET, tcp->pid, NT_ARM_SYSTEM_CALL, &io);
	if (rc && errno != ESRCH)
		perror_func_msg("NT_ARM_SYSTEM_CALL pid:%d scno:%#x",
				tcp->pid, n);
	return rc;
}
