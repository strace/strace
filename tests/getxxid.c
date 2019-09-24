/*
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_getxpid && defined __NR_getxuid && defined __NR_getxgid

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	long id;
	pid_t ppid;

	id = syscall(__NR_getxpid);
	ppid = getppid();
	printf("getxpid() = %ld (ppid %ld)\n", id, (long) ppid);
	printf("getxpid() = %ld (ppid %ld)\n", id, (long) ppid);

	id = syscall(__NR_getxuid);
	printf("getxuid() = %ld (euid %ld)\n", id, id);

	id = syscall(__NR_getxgid);
	printf("getxgid() = %ld (egid %ld)\n", id, id);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getxpid && __NR_getxuid && __NR_getxgid")

#endif
