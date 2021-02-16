/*
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_RT_SIGFRAME_H
# define STRACE_RT_SIGFRAME_H

# include <signal.h>

/* This is a generic definition for compatible architectures. */

typedef struct {
	siginfo_t	info;
	ucontext_t	uc;
	/* more data might follow */
} struct_rt_sigframe;

#endif /* !STRACE_RT_SIGFRAME_H */
