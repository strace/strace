/*
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __i386__
# include "i386/rt_sigframe.h"
#else
# ifndef STRACE_RT_SIGFRAME_H
#  define STRACE_RT_SIGFRAME_H

#  include <signal.h>

typedef struct {
	kernel_ulong_t	pretcode;
	ucontext_t	uc;
	/* more data follows */
} struct_rt_sigframe;

# endif /* !STRACE_RT_SIGFRAME_H */
#endif /* !__i386__ */
