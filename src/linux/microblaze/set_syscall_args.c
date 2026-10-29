/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_syscall_args(struct tcb *tcp, unsigned int arg_mask,
		      kernel_ulong_t args[])
{
	for (unsigned int i = 0; i < MAX_ARGS; ++i) {
		if ((arg_mask & (1U << i)) &&
		    upoke(tcp, (5 + i) * 4, args[i]) != 0)
			return -1;
	}
	return 0;
}
