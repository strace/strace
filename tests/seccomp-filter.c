/*
 * Check decoding of seccomp SECCOMP_SET_MODE_FILTER.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include "scno.h"
#include <unistd.h>

#ifdef HAVE_LINUX_SECCOMP_H
# include <linux/seccomp.h>
#endif
#include <linux/filter.h>

#if defined __NR_seccomp && defined SECCOMP_SET_MODE_FILTER

# define N 7

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
	       " = %ld %s (%m)\n",
	       "SECCOMP_FILTER_FLAG_TSYNC|SECCOMP_FILTER_FLAG_LOG|"
	       "SECCOMP_FILTER_FLAG_SPEC_ALLOW|"
	       "SECCOMP_FILTER_FLAG_NEW_LISTENER|"
	       "SECCOMP_FILTER_FLAG_TSYNC_ESRCH|0xffffffe0",
	       prog->len, prog->filter, rc, errno2name());

	rc = syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, -32L, efault);
	printf("seccomp(SECCOMP_SET_MODE_FILTER, %s, %p) = %ld %s (%m)\n",
	       "0xffffffe0 /* SECCOMP_FILTER_FLAG_??? */",
	       efault, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_seccomp && SECCOMP_SET_MODE_FILTER")

#endif
