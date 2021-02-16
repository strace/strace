/*
 * Copyright (c) 2010-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

int
get_personality_from_syscall_info(const struct_ptrace_syscall_info *sci)
{
	unsigned int pers = sci->arch == AUDIT_ARCH_I386;

#ifndef X32
	switch(sci->op) {
		case PTRACE_SYSCALL_INFO_ENTRY:
		case PTRACE_SYSCALL_INFO_SECCOMP:
			break;
		default:
			return -1;
	}

	kernel_ulong_t scno = sci->entry.nr;

	if (pers == 0 && (scno & __X32_SYSCALL_BIT)) {
		/*
		 * Syscall number -1 requires special treatment:
		 * it might be a side effect of SECCOMP_RET_ERRNO
		 * filtering that sets orig_rax to -1
		 * in some versions of linux kernel.
		 * If that is the case, then
		 * __X32_SYSCALL_BIT logic does not apply.
		 */
		if (scno != (kernel_ulong_t) -1)
			pers = 2;
	}
#endif /* !X32 */

	return pers;
}
