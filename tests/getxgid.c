/*
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_getxgid

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	long id = syscall(__NR_getxgid);
	printf("getxgid() = %ld (egid %ld)\n", id, id);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getxgid")

#endif
