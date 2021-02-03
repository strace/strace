/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

/*
 * Fetch indirect syscall arguments that are provided as an array.
 * Return a pointer to a static array of kernel_ulong_t elements,
 * or NULL in case of fetch failure.
 */
kernel_ulong_t *
fetch_indirect_syscall_args(struct tcb *const tcp,
			    const kernel_ulong_t addr,
			    const unsigned int n_args)
{
	static kernel_ulong_t u_arg[MAX_ARGS];

	if (current_wordsize == sizeof(*u_arg)) {
		if (umoven(tcp, addr, sizeof(*u_arg) * n_args, u_arg))
			return NULL;
	} else {
		uint32_t narrow_arg[ARRAY_SIZE(u_arg)];

		if (umoven(tcp, addr, sizeof(*narrow_arg) * n_args, narrow_arg))
			return NULL;
		for (unsigned int i = 0; i < n_args; ++i)
			u_arg[i] = narrow_arg[i];
	}

	return u_arg;
}
