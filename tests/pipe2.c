/*
 * Check decoding of pipe2 syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_pipe2

# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	int *const fds = tail_alloc(sizeof(*fds) * 2);
	int *const efault = fds + 1;
	long rc;

	rc = syscall(__NR_pipe2, fds, F8ILL_KULONG_MASK | O_NONBLOCK);
	if (rc)
		perror_msg_and_skip("pipe2");
	printf("pipe2([%d, %d], O_NONBLOCK) = 0\n", fds[0], fds[1]);

	rc = syscall(__NR_pipe2, efault, F8ILL_KULONG_MASK);
	printf("pipe2(%p, 0) = %s\n", efault, sprintrc(rc));

	if (F8ILL_KULONG_SUPPORTED) {
		const kernel_ulong_t ill = f8ill_ptr_to_kulong(fds);
		rc = syscall(__NR_pipe2, ill, 0);
		printf("pipe2(%#llx, 0) = %s\n",
		       (unsigned long long) ill, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pipe2")

#endif
