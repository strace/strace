/*
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_SCHED_ATTR_H
# define STRACE_SCHED_ATTR_H

# include <stdint.h>

struct sched_attr {
	uint32_t size;
	uint32_t sched_policy;
	uint64_t sched_flags;
	uint32_t sched_nice;
	uint32_t sched_priority;
	uint64_t sched_runtime;
	uint64_t sched_deadline;
	uint64_t sched_period;
};

# define SCHED_ATTR_MIN_SIZE	48

#endif /* !STRACE_SCHED_ATTR_H */
