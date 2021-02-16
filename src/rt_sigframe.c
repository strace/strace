/*
 * Copyright (c) 2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "ptrace.h"
#include "regs.h"

#define DEF_FUNC_GET_RT_SIGFRAME_ADDR	\
		kernel_ulong_t get_rt_sigframe_addr(struct tcb *tcp)
#define FUNC_GET_RT_SIGFRAME_ADDR	DEF_FUNC_GET_RT_SIGFRAME_ADDR

#include "arch_rt_sigframe.c"
