/*
 * This file is part of time strace test.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_time

# include <errno.h>
# include <stdio.h>
# include <stdint.h>
# include <unistd.h>

typedef kernel_ulong_t kernel_time_t;

static kernel_long_t
k_time(void *p)
{
# if defined __x86_64__ && defined __ILP32__
	register long arg asm("rdi") = (uintptr_t) p;
	kernel_long_t rc;
	asm volatile("syscall\n\t"
		     : "=a"(rc)
		     : "0"(__NR_time), "r"(arg)
		     : "memory", "cc", "r11", "cx");
	if (rc < 0 && rc >= -4095) {
		errno = -rc;
		rc = (kernel_long_t) -1LL;
	}
	return rc;
# else
	const kernel_ulong_t arg = (uintptr_t) p;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	return syscall(__NR_time, arg, bad, bad, bad, bad, bad);
# endif
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_time_t, p);

	kernel_ulong_t t = k_time(NULL);
	if (t == (kernel_ulong_t) -1ULL)
		perror_msg_and_skip("time");
	printf("time(NULL) = %llu (", (unsigned long long) t);
	print_time_t_nsec(t, 0, 0);
	puts(")");

	t = k_time(p + 1);
	printf("time(%p) = %s\n", p + 1, sprintrc(t));

	t = k_time(p);
	printf("time([%lld", (long long) *p);
	print_time_t_nsec(*p, 0, 1),
	printf("]) = %llu (", (unsigned long long) t);
	print_time_t_nsec(t, 0, 0);
	puts(")");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_time")

#endif
