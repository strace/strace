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

#if defined __NR_openat && defined HAVE_SELINUX_RUNTIME

# include <asm/fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

/* for getcwd()/opendir() */
# include <limits.h>
# include <sys/types.h>
# include <dirent.h>

# include "selinux.c"

static const char sample[] = "openat.sample";

int
main(void)
{
	unlink(sample);
	long fd = syscall(__NR_openat, -100, sample, O_RDONLY|O_CREAT, 0400);
	if (fd == -1)
		perror_msg_and_fail("openat");
	close(fd);

	char *sample_realpath = realpath(sample, NULL);
	if (sample_realpath == NULL)
		perror_msg_and_fail("realpath");

	/*
	 * file context in openat() is not displayed because file doesn't exist
	 * yet, but is displayed in return value since the file got created
	 */
	printf("%sopenat(AT_FDCWD, \"%s\", O_RDONLY|O_CREAT, 0400) = %s<%s>%s\n",
	       SELINUX_MYCONTEXT(),
	       sample,
	       sprintrc(fd), sample_realpath, SELINUX_FILECONTEXT(sample));

	fd = syscall(__NR_openat, -100, sample, O_RDONLY);
	printf("%sopenat(AT_FDCWD, \"%s\"%s, O_RDONLY) = %s<%s>%s\n",
	       SELINUX_MYCONTEXT(),
	       sample, SELINUX_FILECONTEXT(sample),
	       sprintrc(fd), sample_realpath, SELINUX_FILECONTEXT(sample));
	if (fd != -1)
		close(fd);

	if (unlink(sample) == -1)
		perror_msg_and_fail("unlink");

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

	fd = syscall(__NR_openat, dfd, sample, O_RDONLY|O_CREAT, 0400);
	if (fd == -1)
		perror_msg_and_fail("openat");
	close(fd);

	/*
	 * file context in openat() is not displayed because file doesn't exist
	 * yet, but is displayed in return value since the file got created
	 */
	printf("%sopenat(%d<%s>%s, \"%s\", O_RDONLY|O_CREAT, 0400) = %s<%s>%s\n",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       sample,
	       sprintrc(fd), sample_realpath, SELINUX_FILECONTEXT(sample));

	fd = syscall(__NR_openat, dfd, sample, O_RDONLY);
	printf("%sopenat(%d<%s>%s, \"%s\"%s, O_RDONLY) = %s<%s>%s\n",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       sample, SELINUX_FILECONTEXT(sample),
	       sprintrc(fd), sample_realpath, SELINUX_FILECONTEXT(sample));
	if (fd != -1)
		close(fd);

	unlink(sample);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_openat && HAVE_SELINUX_RUNTIME")

#endif
