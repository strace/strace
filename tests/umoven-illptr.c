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
#include "scno.h"

#ifdef __NR_nanosleep

# include <stdio.h>
# include <unistd.h>

# include "kernel_old_timespec.h"

static const char *errstr;

static long
k_nanosleep(const kernel_ulong_t arg1, const kernel_ulong_t arg2)
{
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const long rc = syscall(__NR_nanosleep, arg1, arg2, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	if (!F8ILL_KULONG_SUPPORTED)
		return 77;

	kernel_old_timespec_t ts = { 0, 0 };
	const void *const p = tail_memdup(&ts, sizeof(ts));

	k_nanosleep((unsigned long) p, 0);
	printf("nanosleep({tv_sec=0, tv_nsec=0}, NULL) = %s\n", errstr);

	const kernel_ulong_t ill = f8ill_ptr_to_kulong(p);
	k_nanosleep(ill, 0);
	printf("nanosleep(%#llx, NULL) = %s\n",
	       (unsigned long long) ill, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_nanosleep")

#endif
