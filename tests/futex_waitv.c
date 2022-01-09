/*
 * Check decoding of futex_waitv syscall.
 *
 * Copyright (c) 2015-2022 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "kernel_timespec.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <linux/futex.h>

static const char *errstr;

static long
k_futex_waitv(const void *const waiters,
	      const unsigned int nr_futexes,
	      const unsigned int flags,
	      const void *const timeout,
	      const unsigned int clockid)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = (uintptr_t) waiters;
	const kernel_ulong_t arg2 = fill | nr_futexes;
	const kernel_ulong_t arg3 = fill | flags;
	const kernel_ulong_t arg4 = (uintptr_t) timeout;
	const kernel_ulong_t arg5 = fill | clockid;
	const long rc = syscall(__NR_futex_waitv,
				arg1, arg2, arg3, arg4, arg5, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, futex);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct futex_waitv, waiter);
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_timespec64_t, ts);
	ts->tv_sec = 1;
	ts->tv_nsec = 2;

	k_futex_waitv(0, 1, -1U, 0, 1);
	printf("futex_waitv(NULL, 1, %#x, NULL, CLOCK_MONOTONIC) = %s\n",
	       -1U, errstr);

	k_futex_waitv(waiter + 1, 0, 1, ts + 1, -1U);
	printf("futex_waitv([], 0, %#x, %p, %#x /* CLOCK_??? */) = %s\n",
	       1, ts + 1, -1U, errstr);

	k_futex_waitv((void *) waiter + 1, 1, 0, ts, 0);
	printf("futex_waitv(%p, 1, 0, {tv_sec=1, tv_nsec=2}, CLOCK_REALTIME)"
	       " = %s\n",
	       (void *) waiter + 1, errstr);

	waiter->uaddr = 0;
	k_futex_waitv(waiter, 1, 0, 0, 1);
	printf("futex_waitv([{val=%#llx, uaddr=NULL, flags=%s|%#x"
	       ", __reserved=%#x}], 1, 0, NULL, CLOCK_MONOTONIC) = %s\n",
	       (unsigned long long) waiter->val,
	       "FUTEX_32|FUTEX_PRIVATE_FLAG",
	       waiter->flags & ~(FUTEX_32|FUTEX_PRIVATE_FLAG),
	       waiter->__reserved, errstr);

	waiter->val = 0xdeadbeeffacefeedULL;
	waiter->uaddr = -1ULL;
	waiter->flags = 0;
	waiter->__reserved = 0;
	k_futex_waitv(waiter, 1, 0, 0, 2);
	printf("futex_waitv([{val=%#llx, uaddr=%#llx, flags=0}], 1, 0, NULL"
	       ", CLOCK_PROCESS_CPUTIME_ID) = %s\n",
	       (unsigned long long) waiter->val,
	       (unsigned long long) waiter->uaddr,
	       errstr);

	waiter->val = 0;
	waiter->uaddr = (uintptr_t) futex;
	waiter->flags = FUTEX_PRIVATE_FLAG;
	k_futex_waitv(waiter, 1, 0, 0, 0);
	printf("futex_waitv([{val=0, uaddr=%p, flags=%s}], 1, 0, NULL"
	       ", CLOCK_REALTIME) = %s\n",
	       futex, "FUTEX_PRIVATE_FLAG", errstr);

	waiter->flags = FUTEX_32;
	k_futex_waitv(waiter, 2, 0, 0, 1);
	printf("futex_waitv([{val=0, uaddr=%p, flags=%s}, ... /* %p */], 2, 0, NULL"
	       ", CLOCK_MONOTONIC) = %s\n",
	       futex, "FUTEX_32", waiter + 1, errstr);

	waiter->flags = FUTEX_32|FUTEX_PRIVATE_FLAG;
	k_futex_waitv(waiter, 1, 0, ts, 1);
	printf("futex_waitv([{val=0, uaddr=%p, flags=%s}], 1, 0"
	       ", {tv_sec=1, tv_nsec=2}, CLOCK_MONOTONIC) = %s\n",
	       futex, "FUTEX_32|FUTEX_PRIVATE_FLAG", errstr);

	unsigned int nr = FUTEX_WAITV_MAX + 1;
	uint32_t * const futexes = tail_alloc(nr * sizeof(*futexes));
	struct futex_waitv * const waiters = tail_alloc(nr * sizeof(*waiters));
	for (unsigned int i = 0; i < nr; ++i) {
		futexes[i] = i;
		waiters[i].val = i;
		waiters[i].uaddr = (uintptr_t) &futexes[i];
		waiters[i].flags = FUTEX_32|FUTEX_PRIVATE_FLAG;
		waiters[i].__reserved = 0;
	}
	k_futex_waitv(waiters, nr, 0, ts, 1);
	printf("futex_waitv([");
	for (unsigned int i = 0; i < FUTEX_WAITV_MAX; ++i) {
		printf("%s{val=%#x, uaddr=%p, flags=%s}",
		       i ? ", " : "",
		       i, &futexes[i], "FUTEX_32|FUTEX_PRIVATE_FLAG");
	}
	printf(", ...], %u, 0, {tv_sec=1, tv_nsec=2}, CLOCK_MONOTONIC) = %s\n",
	       nr, errstr);

	nr = FUTEX_WAITV_MAX;
	k_futex_waitv(waiters + 1, nr, 0, ts, 1);
	printf("futex_waitv([");
	for (unsigned int i = 0; i < FUTEX_WAITV_MAX; ++i) {
		printf("%s{val=%#x, uaddr=%p, flags=%s}",
		       i ? ", " : "",
		       i + 1, &futexes[i + 1], "FUTEX_32|FUTEX_PRIVATE_FLAG");
	}
	printf("], %u, 0, {tv_sec=1, tv_nsec=2}, CLOCK_MONOTONIC) = %s\n",
	       nr, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
