/*
 * Copyright (c) 2015-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "negated_errno.h"

#ifndef ARCH_REGSET
# define ARCH_REGSET s390_regset
#endif

#ifndef NT_ARM_SYSTEM_CALL
# define NT_S390_SYSTEM_CALL 0x307
#endif

/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	typeof(ARCH_REGSET.gprs[2]) gpr2 = ARCH_REGSET.gprs[2];

	if (is_negated_errno(gpr2)) {
		/*
		 * We are in restart_syscall and gprs[2] is clobbered
		 * by the errno, using pulling the syscall number
		 * via NT_S390_SYSTEM_CALL regset.
		 */
		unsigned int scno;
		const struct iovec io = {
			.iov_base = &scno,
			.iov_len = sizeof(scno)
		};
		int rc = ptrace(PTRACE_GETREGSET, tcp->pid, NT_S390_SYSTEM_CALL,
				&io);
		if (rc && errno != ESRCH) {
			perror_func_msg("NT_S390_SYSTEM_CALL pid:%d", tcp->pid);
			return -1;
		}
		/*
		 * The ptrace call returns thread.system_call, that stores raw
		 * int_code, and the syscall number part is the lowest 16 bits
		 * of it.
		 */
		tcp->scno = scno & 0xffff;
	} else {
		tcp->scno = gpr2 ?: ARCH_REGSET.gprs[1];
	}
	return 1;
}
