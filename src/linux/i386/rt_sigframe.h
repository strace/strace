/*
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_RT_SIGFRAME_H
# define STRACE_RT_SIGFRAME_H

# include <signal.h>

typedef struct {
	uint32_t	pretcode;
	int		sig;
	uint32_t	pinfo;
	uint32_t	puc;
	siginfo_t	info;
	ucontext_t	uc;
	/* more data follows */
} struct_rt_sigframe;

#endif /* !STRACE_RT_SIGFRAME_H */
