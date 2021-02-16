/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "negated_errno.h"

static void
arch_get_error(struct tcb *tcp, const bool check_errno)
{
	/*
	 * In X32, return value is 64-bit (llseek uses one).
	 * Using merely "long rax" would not work.
	 */
	long long rax;

	if (x86_io.iov_len == sizeof(i386_regs)) {
		/* Sign extend from 32 bits */
		rax = (int32_t) i386_regs.eax;
	} else {
		rax = x86_64_regs.rax;
	}

	if (check_errno && is_negated_errno(rax)) {
		tcp->u_rval = -1;
		tcp->u_error = -rax;
	} else {
		if (x86_io.iov_len == sizeof(i386_regs))
			tcp->u_rval = (uint32_t) rax;
		else
			tcp->u_rval = rax;
	}
}
