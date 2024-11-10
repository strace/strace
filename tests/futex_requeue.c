/*
 * Check decoding of futex_requeue syscall.
 *
 * Copyright (c) 2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "futex2_flags.h"
#include "scno.h"
#include "xmalloc.h"

#include <stdio.h>
#include <unistd.h>
#include <linux/futex.h>

static const char *errstr;

static long
k_futex_requeue(const void *const waiters,
		const unsigned int flags,
		const unsigned int nr_wake,
		const unsigned int nr_requeue)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = (uintptr_t) waiters;
	const kernel_ulong_t arg2 = fill | flags;
	const kernel_ulong_t arg3 = fill | nr_wake;
	const kernel_ulong_t arg4 = fill | nr_requeue;
	const long rc = syscall(__NR_futex_requeue,
				arg1, arg2, arg3, arg4, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

static const struct {
	unsigned long val;
	const char *raw;
	const char *abbrev;
	const char *verbose;
} flags0 = {
	ARG_STR(0x82),
	"FUTEX2_SIZE_U32|FUTEX2_PRIVATE",
	"0x82 /* FUTEX2_SIZE_U32|FUTEX2_PRIVATE */"
};

int
main(void)
{
	const unsigned int nr = 2;
	TAIL_ALLOC_OBJECT_CONST_ARR(uint32_t, futexes, nr);
	TAIL_ALLOC_OBJECT_CONST_ARR(struct futex_waitv, waiters, nr);
	fill_memory(waiters, nr * sizeof(*waiters));
	const int flags = 0xfacefed0;
	const int nr_wake = 0xfacefed1;
	const int nr_requeue = 0xfacefed2;

	k_futex_requeue(0, 0, nr_wake, nr_requeue);
	printf("futex_requeue(NULL, 0, %d, %d) = %s\n",
	       nr_wake, nr_requeue, errstr);

	k_futex_requeue(waiters + nr, flags, nr_wake, nr_requeue);
	printf("futex_requeue(%p, %#x, %d, %d) = %s\n",
	       waiters + nr, flags, nr_wake, nr_requeue, errstr);

	for (size_t f = 0; f < ARRAY_SIZE(futex2_flags); ++f) {
		struct strval32 pair[] = {
			{ futex2_flags[f].val, futex2_flags[f].XLAT_name },
			{ flags0.val, flags0.XLAT_name }
		};

		for (unsigned int i = 0; i < nr; ++i) {
			waiters[0].flags = pair[i].val;
			waiters[1].flags = pair[!i].val;

			k_futex_requeue(waiters, flags, nr_wake, nr_requeue);
			printf("futex_requeue([");
			for (unsigned int j = 0; j < nr; ++j) {
				printf("%s{val=%#jx, uaddr=%#jx"
				       ", flags=%s, __reserved=%#x}",
				       j ? ", " : "",
				       (uintmax_t) waiters[j].val,
				       (uintmax_t) waiters[j].uaddr,
				       pair[i ^ j].str,
				       waiters[j].__reserved);
			}
			printf("], %#x, %d, %d) = %s\n",
			       flags, nr_wake, nr_requeue, errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
