/*
 * Check decoding of sethostname syscall.
 *
 * Copyright (c) 2016 Fei Jie <feij.fnst@cn.fujitsu.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>
#include <linux/utsname.h>

int
main(void)
{
	kernel_ulong_t len;
	long rc;

	len = __NEW_UTS_LEN;
	rc = syscall(__NR_sethostname, 0, len);
	printf("sethostname(NULL, %u) = %s\n",
	       (unsigned) len, sprintrc(rc));

	if (F8ILL_KULONG_MASK) {
		len |= F8ILL_KULONG_MASK;
		rc = syscall(__NR_sethostname, 0, len);
		printf("sethostname(NULL, %u) = %s\n",
		       (unsigned) len, sprintrc(rc));
	}

	len = __NEW_UTS_LEN + 1;
	void *const p = tail_alloc(len);
	rc = syscall(__NR_sethostname, p, len);
	printf("sethostname(%p, %u) = %s\n",
	       p, (unsigned) len, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
