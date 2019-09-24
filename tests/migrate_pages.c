/*
 * Check decoding of migrate_pages syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_migrate_pages

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const long pid = (long) 0xfacefeedffffffffULL;
	long rc = syscall(__NR_migrate_pages, pid, 0, 0, 0);
	printf("migrate_pages(%d, 0, NULL, NULL) = %ld %s (%m)\n",
	       (int) pid, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_migrate_pages")

#endif
