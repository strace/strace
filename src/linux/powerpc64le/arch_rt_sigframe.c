/*
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

FUNC_GET_RT_SIGFRAME_ADDR
{
	kernel_ulong_t sp;
	return get_stack_pointer(tcp, &sp) ? sp : 0;
}
