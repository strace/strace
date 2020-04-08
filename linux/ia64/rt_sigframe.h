/*
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_RT_SIGFRAME_H
# define STRACE_RT_SIGFRAME_H

# include <signal.h>

typedef struct {
	unsigned long		arg0;
	unsigned long		arg1;
	unsigned long		arg2;
	void			*handler;
	siginfo_t		info;
	struct sigcontext	sc;
} struct_rt_sigframe;

# define OFFSETOF_SIGMASK_IN_RT_SIGFRAME	\
		offsetof(struct_rt_sigframe, sc.sc_mask)

#endif /* !STRACE_RT_SIGFRAME_H */
