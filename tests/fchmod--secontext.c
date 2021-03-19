/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

/*
 * This test is designed to be executed with the following strace options:
 * --secontext[=full] -y
 */

#if defined __NR_fchmod && defined HAVE_SELINUX_RUNTIME

# include <fcntl.h>
# include <sys/stat.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

# include "selinux.c"

int
main(void)
{
	static const char fname[] = "fchmod_test_file";

	unlink(fname);
	int fd = open(fname, O_CREAT|O_RDONLY, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");

	char *fname_realpath = realpath(fname, NULL);
	if (fname_realpath == NULL)
		perror_msg_and_fail("realpath");

	const char *fname_context = SELINUX_FILECONTEXT(fname);
	long rc = syscall(__NR_fchmod, fd, 0600);
	printf("%sfchmod(%d<%s>%s, 0600) = %s\n",
	       SELINUX_MYCONTEXT(),
	       fd, fname_realpath, fname_context,
	       sprintrc(rc));

	if (unlink(fname))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_fchmod, fd, 0600);
	printf("%sfchmod(%d<%s (deleted)>%s, 0600) = %s\n",
	       SELINUX_MYCONTEXT(),
	       fd, fname_realpath, fname_context,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fchmod && HAVE_SELINUX_RUNTIME")

#endif
