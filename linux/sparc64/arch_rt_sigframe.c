/*
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define STACK_BIAS	2047

FUNC_GET_RT_SIGFRAME_ADDR
{
	kernel_ulong_t sp;
	if (!get_stack_pointer(tcp, &sp))
		return 0;
	return tcp->currpers == 1 ? sp & 0xffffffffUL
				  : sp + STACK_BIAS;
}
