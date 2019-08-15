/*
 * Check decoding of pidfd_open syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_pidfd_open

# include <stdio.h>
# include <unistd.h>
# ifdef PATH_TRACING
#  include <fcntl.h>
# endif

static const char *errstr;

static long
k_pidfd_open(const unsigned int pid, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | pid;
	const kernel_ulong_t arg2 = fill | flags;
	const long rc = syscall(__NR_pidfd_open,
				arg1, arg2, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
# if defined PATH_TRACING || defined PRINT_PATHS
	skip_if_unavailable("/proc/self/fd/");
# endif

# ifdef PATH_TRACING
	static const char path_full[] = "/dev/full";
	(void) close(0);
	if (open(path_full, O_WRONLY))
		perror_msg_and_skip(path_full);
# endif

	k_pidfd_open(0, 0);
# ifndef PATH_TRACING
	printf("pidfd_open(0, 0) = %s\n", errstr);
# endif

	k_pidfd_open(-1U, 0);
# ifndef PATH_TRACING
	printf("pidfd_open(-1, 0) = %s\n", errstr);
# endif

	k_pidfd_open(0, -1U);
# ifndef PATH_TRACING
	printf("pidfd_open(0, %#x) = %s\n", -1U, errstr);
# endif

	const unsigned int flags = 0xfacefeed;
	const int pid = getpid();

	k_pidfd_open(pid, flags);
# ifndef PATH_TRACING
	printf("pidfd_open(%d, %#x) = %s\n", pid, flags, errstr);
# endif

# ifdef PRINT_PATHS
	long rc = k_pidfd_open(pid, 0);
	if (rc < 0)
		perror_msg_and_skip("pidfd_open");
# else
	k_pidfd_open(pid, 0);
# endif

# ifndef PATH_TRACING
	printf("pidfd_open(%d, 0) = "
#  ifdef PRINT_PATHS
	       "%ld<anon_inode:[pidfd]>\n", pid, rc
#  else
	       "%s\n", pid, errstr
#  endif
	       );
# endif

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pidfd_open");

#endif
