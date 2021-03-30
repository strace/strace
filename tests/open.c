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

# include "selinux.c"

int
main(void)
{
	static const char sample[] = "open.sample";
	char *my_secontext = SELINUX_MYCONTEXT();

	long fd = syscall(__NR_open, sample, O_RDONLY|O_CREAT, 0400);
	printf("%sopen(\"%s\", O_RDONLY|O_CREAT, 0400) = %s%s\n",
	       my_secontext,
	       sample, sprintrc(fd), SELINUX_FILECONTEXT(sample));

	if (fd != -1) {
		close(fd);
		if (unlink(sample))
			perror_msg_and_fail("unlink");

		fd = syscall(__NR_open, sample, O_RDONLY);
		printf("%sopen(\"%s\", O_RDONLY) = %s\n",
		       my_secontext, sample, sprintrc(fd));

		fd = syscall(__NR_open, sample, O_WRONLY|O_NONBLOCK|0x80000000);
		printf("%sopen(\"%s\", O_WRONLY|O_NONBLOCK|0x80000000) = %s\n",
		       my_secontext, sample, sprintrc(fd));
	}

# ifdef O_TMPFILE
	fd = syscall(__NR_open, sample, O_WRONLY|O_TMPFILE, 0600);
	printf("%sopen(\"%s\", O_WRONLY|O_TMPFILE, 0600) = %s\n",
	       my_secontext,
	       sample, sprintrc(fd));
# endif /* O_TMPFILE */

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_open")

#endif
