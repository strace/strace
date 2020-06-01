/*
 * Copyright (c) 2019-2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_TIMESPEC_H
# define STRACE_KERNEL_TIMESPEC_H

typedef struct {
	long long tv_sec;
	long long tv_nsec;
} kernel_timespec64_t;

# if HAVE_ARCH_TIME32_SYSCALLS || HAVE_ARCH_TIMESPEC32

typedef struct {
	int tv_sec;
	int tv_nsec;
} kernel_timespec32_t;

# endif /* HAVE_ARCH_TIME32_SYSCALLS || HAVE_ARCH_TIMESPEC32 */

#endif /* !STRACE_KERNEL_TIMESPEC_H */
