/*
 * Copyright (c) 2019-2023 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_TIMESPEC_H
# define STRACE_KERNEL_TIMESPEC_H

# include "arch_defs.h"

typedef struct kernel_timespec64_t {
	long long tv_sec;
	long long tv_nsec;
} kernel_timespec64_t;

# if HAVE_ARCH_TIME32_SYSCALLS || HAVE_ARCH_TIMESPEC32

typedef struct kernel_timespec32_t {
	int tv_sec;
	int tv_nsec;
} kernel_timespec32_t;

# endif /* HAVE_ARCH_TIME32_SYSCALLS || HAVE_ARCH_TIMESPEC32 */

#endif /* !STRACE_KERNEL_TIMESPEC_H */
