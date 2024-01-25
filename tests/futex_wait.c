/*
 * Check decoding of futex_wait syscall.
 *
 * Copyright (c) 2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "futex2_flags.h"
#include "kernel_timespec.h"
#include "scno.h"
#include "xmalloc.h"

#include <stdio.h>
#include <unistd.h>

static const char *errstr;

static long
k_futex_wait(const void *const uaddr,
	     const unsigned long val,
	     const unsigned long mask,
	     const unsigned int flags,
	     const void *const timeout,
	     const unsigned int clockid)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = (uintptr_t) uaddr;
	const kernel_ulong_t arg2 = val;
	const kernel_ulong_t arg3 = mask;
	const kernel_ulong_t arg4 = fill | flags;
	const kernel_ulong_t arg5 = (uintptr_t) timeout;
	const kernel_ulong_t arg6 = fill | clockid;
	const long rc = syscall(__NR_futex_wait, arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

static const struct {
	unsigned long val;
	const char *raw;
	const char *abbrev;
	const char *verbose;
} masks[] = { {
		ARG_STR(0),
		"0",
		"0"
	}, {
		ARG_STR(0xffffffff),
		"FUTEX_BITSET_MATCH_ANY",
		"0xffffffff /* FUTEX_BITSET_MATCH_ANY */"
	}, {
#if SIZEOF_LONG == 4
		ARG_STR(0xfffffff1),
		"0xfffffff1",
		"0xfffffff1"
#else
		0xfffffff1fffffff2UL,
		"0xfffffff1fffffff2",
		"0xfffffff1fffffff2",
		"0xfffffff1fffffff2"
#endif
	},
}, clocks[] = { {
		ARG_STR(0),
		"CLOCK_REALTIME",
		"0 /* CLOCK_REALTIME */"
	}, {
		ARG_STR(0x1),
		"CLOCK_MONOTONIC",
		"0x1 /* CLOCK_MONOTONIC */"
	}, {
		ARG_STR(0xffffffff),
		"0xffffffff /* CLOCK_??? */",
		"0xffffffff /* CLOCK_??? */"
	},
};

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, uaddr);
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_timespec64_t, ts);
	ts->tv_sec = 1;
	ts->tv_nsec = 2;
	const unsigned long val = (unsigned long) 0xfffffff3fffffff4ULL;

	struct {
		void *val;
		const char *str;
	} addrs[] = {
		{ ARG_STR(NULL) },
		{ uaddr, xasprintf("%p", uaddr) },
	}, timeouts[] = {
		{ ARG_STR(NULL) },
		{ ts + 1, xasprintf("%p", ts + 1) },
		{ ts, "{tv_sec=1, tv_nsec=2}" }
	};

	for (size_t a = 0; a < ARRAY_SIZE(addrs); ++a) {
		for (size_t m = 0; m < ARRAY_SIZE(masks); ++m) {
			for (size_t f = 0; f < ARRAY_SIZE(futex2_flags); ++f) {
				for (size_t t = 0; t < ARRAY_SIZE(timeouts); ++t) {
					for (size_t c = 0; c < ARRAY_SIZE(clocks); ++c) {
						k_futex_wait(addrs[a].val,
							     val,
							     masks[m].val,
							     futex2_flags[f].val,
							     timeouts[t].val,
							     clocks[c].val);
						printf("futex_wait(%s, %lu, %s"
						       ", %s, %s, %s) = %s\n",
						       addrs[a].str,
						       val,
						       masks[m].XLAT_name,
						       futex2_flags[f].XLAT_name,
						       timeouts[t].str,
						       clocks[c].XLAT_name,
						       errstr);
					}
				}
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
