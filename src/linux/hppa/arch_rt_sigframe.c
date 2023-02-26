/*
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "rt_sigframe.h"

/* see further comments in GDB: gdb/hppa-linux-tdep.c */

#define SIGFRAME	64

FUNC_GET_RT_SIGFRAME_ADDR
{
	unsigned long sp, ip;

	if (!get_instruction_pointer(tcp, &ip) ||
	    !get_stack_pointer(tcp, &sp))
		return 0;

	sp &= -1UL;
	/* check if ip is part of stack, running in tramp[] of rt_sigframe */
	if ((sp - ip) < 1024) {
		/* on executable stack: We execute in tramp[], so align down. */
		return (ip & -SIGFRAME)
			/* compensate for size difference old and new frame */
			+ sizeof(struct_rt_sigframe_old)
			- sizeof(struct_rt_sigframe);
	} else {
		/* running in VDSO on kernel >= 5.18 */
		static kernel_ulong_t context_offset;

		/* read sigframe offset from kernel VDSO header */
		if (!context_offset)
			context_offset = ptrace(PTRACE_PEEKTEXT, (pid_t) tcp->pid,
						(void *)(ip & -SIGFRAME), 0);
		if (context_offset == (kernel_ulong_t) -1)
			return 0;

		/* context_offset is a negative value */
		return sp + context_offset - offsetof(struct_rt_sigframe, uc.uc_mcontext);
	}
}
