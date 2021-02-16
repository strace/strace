/*
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_KERNEL_OLD_TIMESPEC_H
# define STRACE_KERNEL_OLD_TIMESPEC_H

typedef struct {
# if SIZEOF_KERNEL_LONG_T == 4 || defined LINUX_MIPSN32
	int
# else
	long long
# endif
	tv_sec, tv_nsec;
} kernel_old_timespec_t;

#endif /* !STRACE_KERNEL_OLD_TIMESPEC_H */
