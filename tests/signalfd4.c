/*
 * Check decoding of signalfd4 syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <fcntl.h>
#include "scno.h"

#if defined HAVE_SYS_SIGNALFD_H \
 && defined HAVE_SIGNALFD \
 && defined O_CLOEXEC

# include <signal.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/signalfd.h>

int
main(void)
{
	const char *const sigs = SIGUSR2 < SIGCHLD ? "USR2 CHLD" : "CHLD USR2";
	const unsigned int size = get_sigset_size();

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGCHLD);

	int fd = signalfd(-1, &mask, O_CLOEXEC | O_NONBLOCK);
	printf("signalfd4(-1, [%s], %u, SFD_CLOEXEC|SFD_NONBLOCK) = %s\n",
	       sigs, size, sprintrc(fd));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_SIGNALFD_H && HAVE_SIGNALFD && O_CLOEXEC")

#endif
