/*
 * Check decoding of pidfd_getfd syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_pidfd_getfd

# include <assert.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/wait.h>

# ifndef PIDFD_PATH
#  define PIDFD_PATH ""
# endif
# ifndef FD0_PATH
#  define FD0_PATH ""
# endif
# ifndef PRINT_PIDFD
#  define PRINT_PIDFD 0
# endif

static const char *errstr;

static long
k_pidfd_getfd(const unsigned int pidfd, const unsigned int fd,
	      const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | pidfd;
	const kernel_ulong_t arg2 = fill | fd;
	const kernel_ulong_t arg3 = fill | flags;
	const long rc = syscall(__NR_pidfd_getfd,
				arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	long rc;

	rc = k_pidfd_getfd(-1U, -1U, 0);
	printf("pidfd_getfd(-1, -1, 0) = %s\n", errstr);

	rc = k_pidfd_getfd(0, 0, 0xbadc0ded);
	printf("pidfd_getfd(0" FD0_PATH ", 0, 0xbadc0ded) = %s\n", errstr);

	int child_wait_fds[2];
	if (pipe(child_wait_fds))
		perror_msg_and_fail("pipe");

	int dupfd = dup(0);
	int pid = fork();
	if (pid == 0) {
		close(0);
		close(child_wait_fds[1]);
		if (read(child_wait_fds[0], &child_wait_fds[1], sizeof(int)))
			_exit(2);
		_exit(0);
	}
	close(dupfd);

	int pidfd = syscall(__NR_pidfd_open, pid, 0);
# if PRINT_PIDFD
	char pidfd_str[sizeof("<pid:>") + 3 * sizeof(int)];
	snprintf(pidfd_str, sizeof(pidfd_str), "<pid:%d>", pid);
# else
	const char *pidfd_str = PIDFD_PATH;
# endif
	rc = k_pidfd_getfd(pidfd, dupfd, 0);
	printf("pidfd_getfd(%d%s, %d%s, 0) = %s%s\n",
	       pidfd, pidfd >= 0 ? pidfd_str : "",
	       dupfd, pidfd >= 0 ? FD0_PATH : "",
	       errstr, rc >= 0 ? FD0_PATH : "");

	puts("+++ exited with 0 +++");

	close(child_wait_fds[1]);
	int status;
	assert(wait(&status) == pid);
	assert(status == 0);

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pidfd_getfd");

#endif
