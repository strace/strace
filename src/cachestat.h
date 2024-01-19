/*
 * Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_CACHESTAT_H
# define STRACE_CACHESTAT_H

# include <stdint.h>

struct cachestat_range {
	uint64_t off;
	uint64_t len;
};

struct cachestat {
	uint64_t nr_cache;
	uint64_t nr_dirty;
	uint64_t nr_writeback;
	uint64_t nr_evicted;
	uint64_t nr_recently_evicted;
};

#endif /* STRACE_CACHESTAT_H */
