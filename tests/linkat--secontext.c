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

#if defined __NR_linkat && defined HAVE_SELINUX_RUNTIME

# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

/* for getcwd()/opendir() */
# include <limits.h>
# include <sys/types.h>
# include <dirent.h>

# include "selinux.c"
# include "xmalloc.h"

int
main(void)
{
	static const char sample_1[] = "linkat_sample_old";
	static const char sample_2[] = "linkat_sample_new";
	int fd_sample_1;

	unlink(sample_1);
	unlink(sample_2);
	fd_sample_1 = open(sample_1, O_RDONLY | O_CREAT, 0400);
	if (fd_sample_1 < 0)
		perror_msg_and_fail("open");
	if (close(fd_sample_1))
		perror_msg_and_fail("close");

	char *sample_1_realpath = realpath(sample_1, NULL);
	if (sample_1_realpath == NULL)
		perror_msg_and_fail("realpath");

	long rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, 0);
	/* no context printed for sample_2 since file doesn't exist yet */
	printf("%slinkat(AT_FDCWD, \"%s\"%s, AT_FDCWD, \"%s\", 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       sample_1, SELINUX_FILECONTEXT(sample_1),
	       sample_2,
	       sprintrc(rc));

	rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, 0);
	printf("%slinkat(AT_FDCWD, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       sample_1, SELINUX_FILECONTEXT(sample_1),
	       sample_2, SELINUX_FILECONTEXT(sample_2),
	       sprintrc(rc));

	int fd_sample_2;
	fd_sample_2 = open(sample_2, O_RDONLY | O_CREAT, 0400);
	if (fd_sample_2 < 0)
		perror_msg_and_fail("open");
	if (close(fd_sample_2))
		perror_msg_and_fail("close");

	update_context_type(sample_1, "default_t");

	rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, 0);
	printf("%slinkat(AT_FDCWD, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       sample_1, SELINUX_FILECONTEXT(sample_1),
	       sample_2, SELINUX_FILECONTEXT(sample_2),
	       sprintrc(rc));

	if (unlink(sample_2))
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
	int dfd_old = dirfd(dir);
	if (dfd_old == -1)
		perror_msg_and_fail("dirfd");

	static const char sample_2_dir[] = "new";
	char *new_sample_2 = xasprintf("%s/%s", sample_2_dir, sample_2);

	unlink(new_sample_2);
	rmdir(sample_2_dir);

	int res = mkdir(sample_2_dir, 0700);
	if (res == -1)
		perror_msg_and_fail("mkdir");

	char *sample_2_dir_realpath = realpath(sample_2_dir, NULL);
	if (sample_2_dir_realpath == NULL)
		perror_msg_and_fail("realpath");

	dir = opendir(sample_2_dir);
	if (dir == NULL)
		perror_msg_and_fail("opendir");
	int dfd_new = dirfd(dir);
	if (dfd_new == -1)
		perror_msg_and_fail("dirfd");

	rc = syscall(__NR_linkat, dfd_old, sample_1, -100, sample_2, 0);
	/* no context printed for sample_2 since file doesn't exist yet */
	printf("%slinkat(%d<%s>%s, \"%s\"%s, AT_FDCWD, \"%s\", 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd_old, cwd, SELINUX_FILECONTEXT(cwd),
	       sample_1, SELINUX_FILECONTEXT(sample_1),
	       sample_2,
	       sprintrc(rc));

	rc = syscall(__NR_linkat, dfd_old, sample_1, -100, sample_2, 0);
	printf("%slinkat(%d<%s>%s, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd_old, cwd, SELINUX_FILECONTEXT(cwd),
	       sample_1, SELINUX_FILECONTEXT(sample_1),
	       sample_2, SELINUX_FILECONTEXT(sample_2),
	       sprintrc(rc));

	if (unlink(sample_2))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_linkat, dfd_old, sample_1, dfd_new, sample_2, 0);
	/* no context printed for sample_2 since file doesn't exist yet */
	printf("%slinkat(%d<%s>%s, \"%s\"%s, %d<%s>%s, \"%s\", 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd_old, cwd, SELINUX_FILECONTEXT(cwd),
	       sample_1, SELINUX_FILECONTEXT(sample_1),
	       dfd_new, sample_2_dir_realpath, SELINUX_FILECONTEXT(sample_2_dir_realpath),
	       sample_2,
	       sprintrc(rc));

	rc = syscall(__NR_linkat, dfd_old, sample_1, dfd_new, sample_2, 0);
	printf("%slinkat(%d<%s>%s, \"%s\"%s, %d<%s>%s, \"%s\"%s, 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd_old, cwd, SELINUX_FILECONTEXT(cwd),
	       sample_1, SELINUX_FILECONTEXT(sample_1),
	       dfd_new, sample_2_dir_realpath, SELINUX_FILECONTEXT(sample_2_dir_realpath),
	       sample_2, SELINUX_FILECONTEXT(new_sample_2),
	       sprintrc(rc));

	char *new_sample_2_realpath = realpath(new_sample_2, NULL);
	if (new_sample_2_realpath == NULL)
		perror_msg_and_fail("realpath");

	/* dfd ignored when path is absolute */
	if (chdir("..") == -1)
		perror_msg_and_fail("chdir");

	rc = syscall(__NR_linkat, dfd_old, sample_1, -100, new_sample_2_realpath, 0);
	printf("%slinkat(%d<%s>%s, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       SELINUX_MYCONTEXT(),
	       dfd_old, cwd, SELINUX_FILECONTEXT(cwd),
	       sample_1, SELINUX_FILECONTEXT(sample_1_realpath),
	       new_sample_2_realpath, SELINUX_FILECONTEXT(new_sample_2_realpath),
	       sprintrc(rc));

	if (chdir(cwd) == -1)
		perror_msg_and_fail("chdir");

	unlink(sample_1);
	unlink(sample_2);
	unlink(new_sample_2);
	rmdir(sample_2_dir);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_linkat && HAVE_SELINUX_RUNTIME")

#endif
