/*
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_SCHED_ATTR_H
# define STRACE_SCHED_ATTR_H
# ifndef SCHED_ATTR_SIZE_VER1

#  include <asm/types.h>

struct sched_attr {
	__u32 size;
	__u32 sched_policy;
	__u64 sched_flags;
	__s32 sched_nice;
	__u32 sched_priority;
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;
	/* ver. 1 fields below */
	__u32 sched_util_min;
	__u32 sched_util_max;
};

#  define SCHED_ATTR_SIZE_VER1  56
# endif
# define SCHED_ATTR_MIN_SIZE	48

#endif /* !STRACE_SCHED_ATTR_H */
