/*
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#undef FUNC_GET_RT_SIGFRAME_ADDR
#define FUNC_GET_RT_SIGFRAME_ADDR	\
	static kernel_ulong_t ppc_get_rt_sigframe_addr(struct tcb *tcp)

#include "../powerpc/arch_rt_sigframe.c"

#undef FUNC_GET_RT_SIGFRAME_ADDR
#define FUNC_GET_RT_SIGFRAME_ADDR	DEF_FUNC_GET_RT_SIGFRAME_ADDR

FUNC_GET_RT_SIGFRAME_ADDR
{
	if (tcp->currpers == 1)
		return ppc_get_rt_sigframe_addr(tcp);
	kernel_ulong_t sp;
	return get_stack_pointer(tcp, &sp) ? sp : 0;
}
