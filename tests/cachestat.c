/*
 * Check decoding of cachestat syscall.
 *
 * Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "cachestat.h"

#include <stdio.h>
#include <unistd.h>

#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE skip_if_unavailable("/proc/self/fd/")
#endif

#ifndef FD9_PATH
# define FD9_PATH "</dev/full>"
#endif

#ifdef RETVAL_INJECTED
# define INJ_STR " (INJECTED)\n"
#else
# define INJ_STR "\n"
#endif

#ifndef TRACE_FDS
# define TRACE_FDS 0
#endif
#ifndef TRACE_PATH
# define TRACE_PATH 0
#endif
#define TRACE_FILTERED (TRACE_FDS || TRACE_PATH)

static const char *errstr;

static long
k_cachestat(const unsigned int fd, const void *crange,
	      const void *cstat, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xbadc0ded00000000ULL;
	const kernel_ulong_t arg1 = fill | fd;
	const kernel_ulong_t arg2 = (uintptr_t) crange;
	const kernel_ulong_t arg3 = (uintptr_t) cstat;
	const kernel_ulong_t arg4 = fill | flags;
	const kernel_ulong_t arg5 = fill | 0xdecaffed;
	const kernel_ulong_t arg6 = fill | 0xdeefaced;
	const long rc = syscall(__NR_cachestat,
				arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	k_cachestat(-1, 0, 0, 0);
#if !TRACE_FILTERED
	printf("cachestat(-1, NULL, NULL, 0) = %s" INJ_STR, errstr);
#endif

	k_cachestat(9, 0, 0, -1U);
	printf("cachestat(9%s, NULL, NULL, %#x) = %s" INJ_STR,
	       FD9_PATH, -1U, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct cachestat_range, crange);
	fill_memory(crange, sizeof(*crange));
	const void *const bad_crange = (void *) crange + 1;

	k_cachestat(9, bad_crange, 0, 0);
	printf("cachestat(9%s, %p, NULL, 0) = %s" INJ_STR,
	       FD9_PATH, bad_crange, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct cachestat, cstat);
	fill_memory(cstat, sizeof(*cstat));
	const void *const bad_cstat = (void *) cstat + 1;

	k_cachestat(9, crange, bad_cstat, 0);
	printf("cachestat(9%s, {off=%#llx, len=%llu}, %p, 0) = %s"
	       INJ_STR, FD9_PATH,
	       (unsigned long long) crange->off,
	       (unsigned long long) crange->len,
	       bad_cstat, errstr);

	if (k_cachestat(9, 0, cstat, 0) < 0) {
		printf("cachestat(9%s, NULL, %p, 0) = %s" INJ_STR,
		       FD9_PATH, cstat, errstr);
	} else {
		printf("cachestat(9%s, NULL, {nr_cache=%llu"
		       ", nr_dirty=%llu, nr_writeback=%llu, nr_evicted=%llu"
		       ", nr_recently_evicted=%llu}, 0) = %s" INJ_STR, FD9_PATH,
		       (unsigned long long) cstat->nr_cache,
		       (unsigned long long) cstat->nr_dirty,
		       (unsigned long long) cstat->nr_writeback,
		       (unsigned long long) cstat->nr_evicted,
		       (unsigned long long) cstat->nr_recently_evicted,
		       errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
