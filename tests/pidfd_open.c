/*
 * Check decoding of pidfd_open syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#ifdef __NR_pidfd_open

# include <stdio.h>
# include <unistd.h>
# include <fcntl.h>

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
	PIDNS_TEST_INIT;

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
	pidns_print_leader();
	printf("pidfd_open(0, 0) = %s\n", errstr);
# endif

	k_pidfd_open(-1U, 0);
# ifndef PATH_TRACING
	pidns_print_leader();
	printf("pidfd_open(-1, 0) = %s\n", errstr);
# endif

	k_pidfd_open(0, O_NONBLOCK);
# ifndef PATH_TRACING
	pidns_print_leader();
	printf("pidfd_open(0, PIDFD_NONBLOCK) = %s\n", errstr);
# endif

	k_pidfd_open(-1U, O_NONBLOCK);
# ifndef PATH_TRACING
	pidns_print_leader();
	printf("pidfd_open(-1, PIDFD_NONBLOCK) = %s\n", errstr);
# endif

	k_pidfd_open(0, -1U);
# ifndef PATH_TRACING
	pidns_print_leader();
	printf("pidfd_open(0, PIDFD_NONBLOCK|%#x) = %s\n",
	       -1U & (~O_NONBLOCK), errstr);
# endif

	const unsigned int flags = 0xfacefeed & (~O_NONBLOCK);
	const int pid = getpid();

	k_pidfd_open(pid, flags);
# ifndef PATH_TRACING
	const char *pid_str = pidns_pid2str(PT_TGID);
	pidns_print_leader();
	printf("pidfd_open(%d%s, %#x /* PIDFD_??? */) = %s\n",
		pid, pid_str, flags, errstr);
# endif

# ifdef PRINT_PATHS
	long rc = k_pidfd_open(pid, 0);
	if (rc < 0)
		perror_msg_and_skip("pidfd_open");
# else
	k_pidfd_open(pid, 0);
# endif

# ifndef PATH_TRACING
	pidns_print_leader();
	printf("pidfd_open(%d%s, 0) = "
#  if defined PRINT_PIDFD
	       "%ld<pid:%d>\n", pid, pid_str, rc, pid
#  elif defined PRINT_PATHS
	       "%ld<anon_inode:[pidfd]>\n", pid, pid_str, rc
#  else
	       "%s\n", pid, pid_str, errstr
#  endif
	       );
# endif

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pidfd_open");

#endif
