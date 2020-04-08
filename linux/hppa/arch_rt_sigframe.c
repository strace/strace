/*
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "rt_sigframe.h"

#define SIGFRAME		(sizeof(long) * 16)
#define FUNCTIONCALLFRAME	(sizeof(long) * 12)
#define RT_SIGFRAME_SIZE	\
	(((sizeof(struct_rt_sigframe) + FUNCTIONCALLFRAME)) & -SIGFRAME)

FUNC_GET_RT_SIGFRAME_ADDR
{
	unsigned long addr;

	return get_stack_pointer(tcp, &addr)
		? (addr & ~1UL) - RT_SIGFRAME_SIZE : 0;
}
