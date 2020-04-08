/*
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_RT_SIGFRAME_H
# define STRACE_RT_SIGFRAME_H

# include <signal.h>

# ifdef __s390x__
#  define SIGNAL_FRAMESIZE 160
# else /* __s390__ */
#  define SIGNAL_FRAMESIZE 96
# endif

typedef struct {
	uint8_t		callee_used_stack[SIGNAL_FRAMESIZE];
	uint16_t	svc_insn;
	siginfo_t	info;
	ucontext_t	uc;
} struct_rt_sigframe;

#endif /* !STRACE_RT_SIGFRAME_H */
