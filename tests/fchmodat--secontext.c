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

#if defined __NR_fchmodat && defined HAVE_SELINUX_RUNTIME

# include <fcntl.h>
# include <limits.h>
# include <sys/stat.h>
# include <stdio.h>
# include <unistd.h>

/* for getcwd()/opendir() */
# include <limits.h>
# include <sys/types.h>
# include <dirent.h>

# include "selinux.c"

int
main(void)
{
	static const char sample[] = "fchmodat_sample";

	unlink(sample);
	if (open(sample, O_RDONLY | O_CREAT, 0400) < 0)
		perror_msg_and_fail("open");

	char *sample_realpath = realpath(sample, NULL);
	if (sample_realpath == NULL)
		perror_msg_and_fail("realpath");

	long rc = syscall(__NR_fchmodat, -100, sample, 0600);
	printf("%sfchmodat(AT_FDCWD, \"%s\"%s, 0600) = %s\n",
	       SELINUX_MYCONTEXT(),
	       sample, SELINUX_FILECONTEXT(sample),
	       sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_fchmodat, -100, sample, 0400);
	printf("%sfchmodat(AT_FDCWD, \"%s\", 0400) = %s\n",
	       SELINUX_MYCONTEXT(),
	       sample,
	       sprintrc(rc));

	/*
	 * Tests with dirfd
	 */

	char cwd[PATH_MAX + 1];
	DIR *dir = NULL;
	if (getcwd(cwd, sizeof (cwd)) == NULL)
		perror_msg_and_fail("getcwd");
	dir = opendir(cwd);
	if (dir == NULL)
		perror_msg_and_fail("opendir");
	int dfd = dirfd(dir);
	if (dfd == -1)
		perror_msg_and_fail("dirfd");

	/* no file */
	rc = syscall(__NR_fchmodat, dfd, sample, 0400);
	printf("%sfchmodat(%d<%s>%s, \"%s\", 0400) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       sample,
	       sprintrc(rc));

	if (open(sample, O_RDONLY | O_CREAT, 0400) < 0)
		perror_msg_and_fail("open");

	rc = syscall(__NR_fchmodat, dfd, sample, 0400);
	printf("%sfchmodat(%d<%s>%s, \"%s\"%s, 0400) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       sample, SELINUX_FILECONTEXT(sample),
	       sprintrc(rc));

	/* dfd ignored when path is absolute */
	if (chdir("..") == -1)
		perror_msg_and_fail("chdir");

	rc = syscall(__NR_fchmodat, dfd, sample_realpath, 0400);
	printf("%sfchmodat(%d<%s>%s, \"%s\"%s, 0400) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       sample_realpath, SELINUX_FILECONTEXT(sample_realpath),
	       sprintrc(rc));

	if (chdir(cwd) == -1)
		perror_msg_and_fail("chdir");

	unlink(sample);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fchmodat && HAVE_SELINUX_RUNTIME")

#endif
