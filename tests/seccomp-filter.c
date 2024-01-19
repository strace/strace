/*
 * Check decoding of seccomp SECCOMP_SET_MODE_FILTER.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <stdio.h>
#include <unistd.h>
#include <linux/seccomp.h>
#include <linux/filter.h>

#define N 7

int
main(void)
{
	struct sock_filter *const filter = tail_alloc(sizeof(*filter) * N);
	const void *const efault = tail_alloc(1);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct sock_fprog, prog);
	long rc;

	prog->filter = filter;
	prog->len = N;
	rc = syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, -1, prog);
	printf("seccomp(SECCOMP_SET_MODE_FILTER, %s, {len=%u, filter=%p})"
	       " = %s\n",
	       "SECCOMP_FILTER_FLAG_TSYNC|SECCOMP_FILTER_FLAG_LOG|"
	       "SECCOMP_FILTER_FLAG_SPEC_ALLOW|"
	       "SECCOMP_FILTER_FLAG_NEW_LISTENER|"
	       "SECCOMP_FILTER_FLAG_TSYNC_ESRCH|"
	       "SECCOMP_FILTER_FLAG_WAIT_KILLABLE_RECV|"
	       "0xffffffc0",
	       prog->len, prog->filter, sprintrc(rc));

	rc = syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, -64L, efault);
	printf("seccomp(SECCOMP_SET_MODE_FILTER, %s, %p) = %s\n",
	       "0xffffffc0 /* SECCOMP_FILTER_FLAG_??? */",
	       efault, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
