/*
 * Copyright (c) 2019-2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_TIMEVAL_H
# define STRACE_KERNEL_TIMEVAL_H

# include "kernel_types.h"

typedef struct {
	long long tv_sec;
	long long tv_usec;
} kernel_timeval64_t;

typedef struct {
	kernel_long_t tv_sec;
# if defined __sparc__ && defined __arch64__
	int tv_usec;
# else
	kernel_long_t tv_usec;
# endif
} kernel_old_timeval_t;

#endif /* !STRACE_KERNEL_TIMEVAL_H */
