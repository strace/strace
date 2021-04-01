/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_open

# include <asm/fcntl.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	/*
	 * Make sure the current workdir of the tracee
	 * is different from the current workdir of the tracer.
	 */
	create_and_enter_subdir("open_subdir");

	static const char sample[] = "open.sample";

	long fd = syscall(__NR_open, sample, O_RDONLY|O_CREAT, 0400);
	printf("open(\"%s\", O_RDONLY|O_CREAT, 0400) = %s\n",
	       sample, sprintrc(fd));

	if (fd != -1) {
		close(fd);
		if (unlink(sample))
			perror_msg_and_fail("unlink");

		fd = syscall(__NR_open, sample, O_RDONLY);
		printf("open(\"%s\", O_RDONLY) = %s\n", sample, sprintrc(fd));

		fd = syscall(__NR_open, sample, O_WRONLY|O_NONBLOCK|0x80000000);
		printf("open(\"%s\", O_WRONLY|O_NONBLOCK|0x80000000) = %s\n",
		       sample, sprintrc(fd));
	}

# ifdef O_TMPFILE
	fd = syscall(__NR_open, sample, O_WRONLY|O_TMPFILE, 0600);
	printf("open(\"%s\", O_WRONLY|O_TMPFILE, 0600) = %s\n",
	       sample, sprintrc(fd));
# endif /* O_TMPFILE */

	leave_and_remove_subdir();

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_open")

#endif
