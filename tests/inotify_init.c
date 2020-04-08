/*
 * Check decoding of inotify_init syscall.
 *
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_inotify_init

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
# ifdef PRINT_PATHS
	skip_if_unavailable("/proc/self/fd/");
# endif

	long rc = syscall(__NR_inotify_init, 42);

# ifdef PRINT_PATHS
	if (rc < 0)
		perror_msg_and_skip("inotify_init");
# endif

	printf("inotify_init() = "
# ifdef PRINT_PATHS
	       "%ld<anon_inode:inotify>\n", rc
# else
	       "%s\n", sprintrc(rc)
# endif
	       );

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_inotify_init");

#endif
