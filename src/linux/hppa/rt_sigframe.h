/*
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_RT_SIGFRAME_H
# define STRACE_RT_SIGFRAME_H

# include <signal.h>

typedef struct {
	unsigned int	tramp[9];
	siginfo_t	info;
	ucontext_t	uc;
} struct_rt_sigframe_old; /* when executed on stack */

typedef struct {
	unsigned int	tramp[2]; /* holds original return address */
	siginfo_t	info;
	ucontext_t	uc;
} struct_rt_sigframe;	 /* when VDSO is used */

#endif /* !STRACE_RT_SIGFRAME_H */
