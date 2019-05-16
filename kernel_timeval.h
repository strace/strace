/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_TIMEVAL_H
# define STRACE_KERNEL_TIMEVAL_H

typedef struct {
	long long tv_sec;
	long long tv_usec;
} kernel_timeval64_t;

#endif /* !STRACE_KERNEL_TIMEVAL_H */
