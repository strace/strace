/*
 * Check decoding of setitimer and getitimer syscalls.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include "scno.h"
#include "kernel_timeval.h"

typedef struct {
	kernel_old_timeval_t it_interval;
	kernel_old_timeval_t it_value;
} kernel_old_itimerval_t;

int
main(void)
{
	static const kernel_old_itimerval_t new = {
		.it_interval = { 0xc0de1, 0xc0de2 },
		.it_value = { 0xc0de3, 0xc0de4 }
	};
	static const kernel_ulong_t long_timer =
		F8ILL_KULONG_MASK | ITIMER_REAL;
	static const kernel_ulong_t bogus_timer =
		(kernel_ulong_t) 0xfacefeeddeadbeefULL;

	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_old_itimerval_t, p_old);
	kernel_old_itimerval_t *const p_new = tail_memdup(&new, sizeof(new));
	void *const efault = tail_alloc(sizeof(new) - 8);
	long rc;

	if (syscall(__NR_setitimer, ITIMER_REAL, p_new, NULL))
		perror_msg_and_skip("setitimer");
	printf("setitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}"
	       ", NULL) = 0\n",
	       (long long) new.it_interval.tv_sec,
	       zero_extend_signed_to_ull(new.it_interval.tv_usec),
	       (long long) new.it_value.tv_sec,
	       zero_extend_signed_to_ull(new.it_value.tv_usec));

	fill_memory(p_old, sizeof(*p_old));
	if (syscall(__NR_getitimer, ITIMER_REAL, p_old))
		perror_msg_and_skip("getitimer");
	printf("getitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}) = 0\n",
	       (long long) p_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_interval.tv_usec),
	       (long long) p_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_value.tv_usec));

	fill_memory(p_old, sizeof(*p_old));
	syscall(__NR_setitimer, ITIMER_REAL, p_new, p_old);
	printf("setitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}) = 0\n",
	       (long long) new.it_interval.tv_sec,
	       zero_extend_signed_to_ull(new.it_interval.tv_usec),
	       (long long) new.it_value.tv_sec,
	       zero_extend_signed_to_ull(new.it_value.tv_usec),
	       (long long) p_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_interval.tv_usec),
	       (long long) p_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_value.tv_usec));

	rc = syscall(__NR_getitimer, ITIMER_REAL, efault);
	printf("getitimer(ITIMER_REAL, %p) = %s\n", efault, sprintrc(rc));

	rc = syscall(__NR_setitimer, ITIMER_REAL, p_new, efault);
	printf("setitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}, %p) = %s\n",
	       (long long) new.it_interval.tv_sec,
	       zero_extend_signed_to_ull(new.it_interval.tv_usec),
	       (long long) new.it_value.tv_sec,
	       zero_extend_signed_to_ull(new.it_value.tv_usec),
	       efault, sprintrc(rc));

	rc = syscall(__NR_setitimer, ITIMER_REAL, efault, p_old);
	printf("setitimer(ITIMER_REAL, %p, %p) = %s\n",
	       efault, p_old, sprintrc(rc));

	fill_memory(p_old, sizeof(*p_old));
	rc = syscall(__NR_setitimer, long_timer, p_new, p_old);
	printf("setitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}) = %s\n",
	       (long long) new.it_interval.tv_sec,
	       zero_extend_signed_to_ull(new.it_interval.tv_usec),
	       (long long) new.it_value.tv_sec,
	       zero_extend_signed_to_ull(new.it_value.tv_usec),
	       (long long) p_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_interval.tv_usec),
	       (long long) p_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_value.tv_usec),
	       sprintrc(rc));

	fill_memory(p_old, sizeof(*p_old));
	rc = syscall(__NR_getitimer, long_timer, p_old);
	printf("getitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}) = %s\n",
	       (long long) p_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_interval.tv_usec),
	       (long long) p_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_value.tv_usec),
	       sprintrc(rc));

	rc = syscall(__NR_setitimer, bogus_timer, p_new, p_old);
	printf("setitimer(%#x /* ITIMER_??? */"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}, %p) = %s\n",
	       (int) bogus_timer,
	       (long long) new.it_interval.tv_sec,
	       zero_extend_signed_to_ull(new.it_interval.tv_usec),
	       (long long) new.it_value.tv_sec,
	       zero_extend_signed_to_ull(new.it_value.tv_usec),
	       p_old, sprintrc(rc));

	rc = syscall(__NR_getitimer, bogus_timer, p_old);
	printf("getitimer(%#x /* ITIMER_??? */, %p) = %s\n",
	       (int) bogus_timer, p_old, sprintrc(rc));

	if (F8ILL_KULONG_SUPPORTED) {
		const kernel_ulong_t ill_new = f8ill_ptr_to_kulong(p_new);
		const kernel_ulong_t ill_old = f8ill_ptr_to_kulong(p_old);

		rc = syscall(__NR_setitimer, long_timer, ill_new, ill_old);
		printf("setitimer(ITIMER_REAL, %#llx, %#llx) = %s\n",
		       (unsigned long long) ill_new,
		       (unsigned long long) ill_old,
		       sprintrc(rc));

		rc = syscall(__NR_getitimer, long_timer, ill_old);
		printf("getitimer(ITIMER_REAL, %#llx) = %s\n",
		       (unsigned long long) ill_old, sprintrc(rc));
	}

	p_new->it_interval.tv_sec = 0xdeadbeefU;
	p_new->it_interval.tv_usec = 0xfacefeedU;
	p_new->it_value.tv_sec =
		(typeof(p_new->it_value.tv_sec)) 0xcafef00ddeadbeefLL;
	p_new->it_value.tv_usec =
		(typeof(p_new->it_value.tv_usec)) 0xbadc0dedfacefeedLL;

	rc = syscall(__NR_setitimer, ITIMER_REAL, p_new, p_old);
	printf("setitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}, %p) = %s\n",
	       (long long) p_new->it_interval.tv_sec,
	       zero_extend_signed_to_ull(p_new->it_interval.tv_usec),
	       (long long) p_new->it_value.tv_sec,
	       zero_extend_signed_to_ull(p_new->it_value.tv_usec),
	       p_old, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
