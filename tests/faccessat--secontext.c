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

#if defined __NR_faccessat && defined HAVE_SELINUX_RUNTIME

# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

/* for getcwd()/opendir() */
# include <limits.h>
# include <sys/types.h>
# include <dirent.h>

# include "selinux.c"

int
main(void)
{
	static const char sample[] = "access_sample";

	unlink(sample);
	int fd = open(sample, O_CREAT|O_RDONLY, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");
	close(fd);

	char *sample_realpath = realpath(sample, NULL);
	if (sample_realpath == NULL)
		perror_msg_and_fail("realpath");

	long rc = syscall(__NR_faccessat, -100, sample, F_OK);
	printf("%sfaccessat(AT_FDCWD, \"%s\"%s, F_OK) = %s\n",
	       SELINUX_MYCONTEXT(),
	       sample, SELINUX_FILECONTEXT(sample),
	       sprintrc(rc));

	if (unlink(sample) == -1)
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_faccessat, -100, sample, F_OK);
	printf("%sfaccessat(AT_FDCWD, \"%s\", F_OK) = %s\n",
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
	rc = syscall(__NR_faccessat, dfd, sample, F_OK);
	printf("%sfaccessat(%d<%s>%s, \"%s\", F_OK) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       sample,
	       sprintrc(rc));

	fd = open(sample, O_CREAT|O_RDONLY, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");
	close(fd);

	rc = syscall(__NR_faccessat, dfd, sample, F_OK);
	printf("%sfaccessat(%d<%s>%s, \"%s\"%s, F_OK) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       sample, SELINUX_FILECONTEXT(sample),
	       sprintrc(rc));

	/* dfd ignored when path is absolute */
	if (chdir("..") == -1)
		perror_msg_and_fail("chdir");

	rc = syscall(__NR_faccessat, dfd, sample_realpath, F_OK);
	printf("%sfaccessat(%d<%s>%s, \"%s\"%s, F_OK) = %s\n",
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

SKIP_MAIN_UNDEFINED("__NR_faccessat && HAVE_SELINUX_RUNTIME")

#endif
