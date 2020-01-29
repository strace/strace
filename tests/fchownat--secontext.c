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

#if defined __NR_fchownat && defined HAVE_SELINUX_RUNTIME

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
	static const char sample[] = "fchownat_sample";
	uid_t uid = geteuid();
	uid_t gid = getegid();

	unlink(sample);
	int fd = open(sample, O_RDONLY | O_CREAT, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");
	close(fd);

	char *sample_realpath = realpath(sample, NULL);
	if (sample_realpath == NULL)
		perror_msg_and_fail("realpath");

	long rc = syscall(__NR_fchownat, -100, sample, uid, gid, 0);
	printf("%sfchownat(AT_FDCWD, \"%s\"%s, %d, %d, 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       sample, SELINUX_FILECONTEXT(sample),
	       uid, gid,
	       sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	/* no file */
	rc = syscall(__NR_fchownat, -100, sample, uid, gid, 0);
	printf("%sfchownat(AT_FDCWD, \"%s\", %d, %d, 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       sample,
	       uid, gid,
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
	rc = syscall(__NR_fchownat, dfd, sample, uid, gid, 0);
	printf("%sfchownat(%d<%s>%s, \"%s\", %d, %d, 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       sample,
	       uid, gid,
	       sprintrc(rc));

	fd = open(sample, O_RDONLY | O_CREAT, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");
	close(fd);

	rc = syscall(__NR_fchownat, dfd, sample, uid, gid, 0);
	printf("%sfchownat(%d<%s>%s, \"%s\"%s, %d, %d, 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       sample, SELINUX_FILECONTEXT(sample),
	       uid, gid,
	       sprintrc(rc));

	/* dfd ignored when path is absolute */
	if (chdir("..") == -1)
		perror_msg_and_fail("chdir");

	rc = syscall(__NR_fchownat, dfd, sample_realpath, uid, gid, 0);
	printf("%sfchownat(%d<%s>%s, \"%s\"%s, %d, %d, 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       sample_realpath, SELINUX_FILECONTEXT(sample_realpath),
	       uid, gid,
	       sprintrc(rc));

	if (chdir(cwd) == -1)
		perror_msg_and_fail("chdir");

	unlink(sample);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fchownat && HAVE_SELINUX_RUNTIME")

#endif
