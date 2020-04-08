/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <fcntl.h>
#include "scno.h"

#if defined __NR_userfaultfd && defined O_CLOEXEC

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	long rc = syscall(__NR_userfaultfd, 1 | O_NONBLOCK | O_CLOEXEC);
	printf("userfaultfd(O_NONBLOCK|O_CLOEXEC|0x1) = %ld %s (%m)\n",
	       rc, errno2name());
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_userfaultfd && O_CLOEXEC")

#endif
