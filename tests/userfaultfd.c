/*
 * Check decoding of userfaultfd syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>
#include "kernel_fcntl.h"

#define UFFD_USER_MODE_ONLY 1

static const char *errstr;

static long
k_userfaultfd(const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | flags;
	const long rc = syscall(__NR_userfaultfd, arg1, bad, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const struct strval32 flags[] = {
		{ ARG_STR(0) },
		{ ARG_STR(O_NONBLOCK) },
		{ ARG_STR(O_CLOEXEC) },
		{ ARG_STR(O_NONBLOCK|O_CLOEXEC) },
		{ ARG_STR(UFFD_USER_MODE_ONLY) },
		{ ARG_STR(UFFD_USER_MODE_ONLY|O_NONBLOCK) },
		{ ARG_STR(UFFD_USER_MODE_ONLY|O_CLOEXEC) },
		{ ARG_STR(UFFD_USER_MODE_ONLY|O_NONBLOCK|O_CLOEXEC) },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(flags); ++i) {
		k_userfaultfd(flags[i].val);
		printf("userfaultfd(%s) = %s\n", flags[i].str, errstr);
	}

	k_userfaultfd(-1U);
	printf("userfaultfd(UFFD_USER_MODE_ONLY|O_NONBLOCK|O_CLOEXEC|%#x)"
	       " = %s\n", (-1U - UFFD_USER_MODE_ONLY - O_NONBLOCK - O_CLOEXEC),
	       errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
