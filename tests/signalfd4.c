/*
 * Check decoding of signalfd4 syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined HAVE_SYS_SIGNALFD_H \
 && defined HAVE_SIGNALFD

# include <signal.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/signalfd.h>
# include "kernel_fcntl.h"

# ifndef SKIP_IF_PROC_IS_UNAVAILABLE
#  define SKIP_IF_PROC_IS_UNAVAILABLE
# endif

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	const char *const sigs1 = "USR2";
	const char *const sigs2 = SIGUSR2 < SIGCHLD ? "USR2 CHLD" : "CHLD USR2";
	const unsigned int size = get_sigset_size();

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);

	int fd = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);

# ifdef PRINT_SIGNALFD
	if (fd == -1)
		perror_msg_and_skip("signalfd");
# endif

	printf("signalfd4(-1, [%s], %u, SFD_CLOEXEC|SFD_NONBLOCK) = %s",
	       sigs1, size, sprintrc(fd));
# ifdef PRINT_SIGNALFD
	printf("<signalfd:[%s]>\n", sigs1);
# else
	putchar('\n');
# endif

	sigaddset(&mask, SIGCHLD);
	fd = signalfd(fd, &mask, 0);

# ifdef PRINT_SIGNALFD
	printf("signalfd4(%d<signalfd:[%s]>, [%s], %u, 0) = %s<signalfd:[%s]>\n",
	       fd, sigs1, sigs2, size, sprintrc(fd), sigs2);
# else
	printf("signalfd4(%d, [%s], %u, 0) = %s\n", fd, sigs2, size, sprintrc(fd));
# endif

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_SIGNALFD_H && HAVE_SIGNALFD")

#endif
