/*
 * Check decoding of invalid pointer by umoven.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "scno.h"

int
main(void)
{
	if (F8ILL_KULONG_SUPPORTED) {
		struct timespec ts = { 0, 0 };
		const void *const p = tail_memdup(&ts, sizeof(ts));

		long rc = syscall(__NR_nanosleep, p, NULL);
		printf("nanosleep({tv_sec=0, tv_nsec=0}, NULL) = %s\n",
		       sprintrc(rc));

		const kernel_ulong_t ill = f8ill_ptr_to_kulong(p);
		rc = syscall(__NR_nanosleep, ill, NULL);
		printf("nanosleep(%#llx, NULL) = %s\n",
		       (unsigned long long) ill, sprintrc(rc));

		puts("+++ exited with 0 +++");
		return 0;
	} else {
		return 77;
	}
}
