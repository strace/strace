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

# include "secontext.h"

int
main(void)
{
	/*
	 * Make sure the current workdir of the tracee
	 * is different from the current workdir of the tracer.
	 */
	create_and_enter_subdir("open_subdir");

	static const char sample[] = "open.sample";
	char *my_secontext = SECONTEXT_PID_MY();

	long fd = syscall(__NR_open, sample, O_RDONLY|O_CREAT, 0400);
	printf("%s%s(\"%s\", O_RDONLY|O_CREAT, 0400) = %s%s\n",
	       my_secontext, "open",
	       sample, sprintrc(fd), SECONTEXT_FILE(sample));

	if (fd != -1) {
		close(fd);
		if (unlink(sample))
			perror_msg_and_fail("unlink");

		fd = syscall(__NR_open, sample, O_RDONLY);
		printf("%s%s(\"%s\", O_RDONLY) = %s\n",
		       my_secontext, "open", sample, sprintrc(fd));

		fd = syscall(__NR_open, sample, O_WRONLY|O_NONBLOCK|0x80000000);
		printf("%s%s(\"%s\", O_WRONLY|O_NONBLOCK|0x80000000) = %s\n",
		       my_secontext, "open", sample, sprintrc(fd));
	}

# ifdef O_TMPFILE
	fd = syscall(__NR_open, sample, O_WRONLY|O_TMPFILE, 0600);
	printf("%s%s(\"%s\", O_WRONLY|O_TMPFILE, 0600) = %s\n",
	       my_secontext, "open",
	       sample, sprintrc(fd));
# endif /* O_TMPFILE */

	leave_and_remove_subdir();

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_open")

#endif
