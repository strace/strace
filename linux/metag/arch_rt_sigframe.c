/*
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "rt_sigframe.h"

FUNC_GET_RT_SIGFRAME_ADDR
{
	kernel_ulong_t sp;
	return get_stack_pointer(tcp, &sp)
		? sp - sizeof(struct_rt_sigframe) : 0;
}
