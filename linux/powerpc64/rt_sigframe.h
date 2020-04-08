/*
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __powerpc64__
# include "../rt_sigframe.h"
#else
# ifndef STRACE_RT_SIGFRAME_H
#  define STRACE_RT_SIGFRAME_H

#  include <signal.h>

typedef struct {
	ucontext_t	uc;
	/* more data follows */
} struct_rt_sigframe;

# endif /* !STRACE_RT_SIGFRAME_H */
#endif /* !__powerpc64__ */
