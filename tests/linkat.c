/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_linkat

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "selinux.c"
#include "xmalloc.h"

int
main(void)
{
	static const char sample_1[] = "linkat_sample_old";
	static const char sample_2[] = "linkat_sample_new";
	const long fd_old = (long) 0xdeadbeefffffffffULL;
	const long fd_new = (long) 0xdeadbeeffffffffeULL;

	char *my_secontext = SELINUX_MYCONTEXT();

	long rc = syscall(__NR_linkat, fd_old, sample_1, fd_new, sample_2, 0);
	printf("%slinkat(%d, \"%s\", %d, \"%s\", 0) = %ld %s (%m)\n",
	       my_secontext,
	       (int) fd_old, sample_1, (int) fd_new, sample_2,
	       rc, errno2name());

	rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, -1L);
	printf("%slinkat(%s, \"%s\", %s, \"%s\", %s) = %ld %s (%m)\n",
	       my_secontext,
	       "AT_FDCWD", sample_1, "AT_FDCWD", sample_2,
	       "AT_SYMLINK_NOFOLLOW|AT_REMOVEDIR|AT_SYMLINK_FOLLOW"
	       "|AT_NO_AUTOMOUNT|AT_EMPTY_PATH|AT_RECURSIVE|0xffff60ff",
	       rc, errno2name());

	int fd_sample_1;

	unlink(sample_1);
	unlink(sample_2);
	fd_sample_1 = open(sample_1, O_RDONLY | O_CREAT, 0400);
	if (fd_sample_1 < 0)
		perror_msg_and_fail("open");
	if (close(fd_sample_1))
		perror_msg_and_fail("close");

	char *sample_1_secontext = SELINUX_FILECONTEXT(sample_1);

	rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, 0);
	/* no context printed for sample_2 since file doesn't exist yet */
	printf("%slinkat(AT_FDCWD, \"%s\"%s, AT_FDCWD, \"%s\", 0) = %s\n",
	       my_secontext,
	       sample_1, sample_1_secontext,
	       sample_2,
	       sprintrc(rc));

	char *sample_2_secontext = sample_1_secontext;

	rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, 0);
	printf("%slinkat(AT_FDCWD, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       my_secontext,
	       sample_1, sample_1_secontext,
	       sample_2, sample_2_secontext,
	       sprintrc(rc));

	int fd_sample_2;
	fd_sample_2 = open(sample_2, O_RDONLY | O_CREAT, 0400);
	if (fd_sample_2 < 0)
		perror_msg_and_fail("open");
	if (close(fd_sample_2))
		perror_msg_and_fail("close");

	free(sample_1_secontext);
	update_context_type(sample_1, "default_t");
	sample_1_secontext = SELINUX_FILECONTEXT(sample_1);
	sample_2_secontext = sample_1_secontext;

	rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, 0);
	printf("%slinkat(AT_FDCWD, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       my_secontext,
	       sample_1, sample_1_secontext,
	       sample_2, sample_2_secontext,
	       sprintrc(rc));

	if (unlink(sample_2))
		perror_msg_and_fail("unlink");

	/*
	 * Tests with dirfd
	 */

	char *cwd = NULL;
	int dfd_old = get_curdir_fd(&cwd);
	char *dfd_old_secontext = SELINUX_FILECONTEXT(".");

	static const char sample_2_dir[] = "new";
	char *new_sample_2 = xasprintf("%s/%s", sample_2_dir, sample_2);

	unlink(new_sample_2);
	rmdir(sample_2_dir);

	int res = mkdir(sample_2_dir, 0700);
	if (res == -1)
		perror_msg_and_fail("mkdir");
	char *sample_2_dir_realpath = xasprintf("%s/%s", cwd, sample_2_dir);
	char *sample_2_dir_secontext = SELINUX_FILECONTEXT(sample_2_dir);

	int dfd_new = get_dir_fd(sample_2_dir);

	rc = syscall(__NR_linkat, dfd_old, sample_1, -100, sample_2, 0);
	/* no context printed for sample_2 since file doesn't exist yet */
	printf("%slinkat(%d%s, \"%s\"%s, AT_FDCWD, \"%s\", 0) = %s\n",
	       my_secontext,
	       dfd_old, dfd_old_secontext,
	       sample_1, sample_1_secontext,
	       sample_2,
	       sprintrc(rc));

	rc = syscall(__NR_linkat, dfd_old, sample_1, -100, sample_2, 0);
	printf("%slinkat(%d%s, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       my_secontext,
	       dfd_old, dfd_old_secontext,
	       sample_1, sample_1_secontext,
	       sample_2, sample_2_secontext,
	       sprintrc(rc));

	if (unlink(sample_2))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_linkat, dfd_old, sample_1, dfd_new, sample_2, 0);
	/* no context printed for sample_2 since file doesn't exist yet */
	printf("%slinkat(%d%s, \"%s\"%s, %d%s, \"%s\", 0) = %s\n",
	       my_secontext,
	       dfd_old, dfd_old_secontext,
	       sample_1, sample_1_secontext,
	       dfd_new, sample_2_dir_secontext,
	       sample_2,
	       sprintrc(rc));

	rc = syscall(__NR_linkat, dfd_old, sample_1, dfd_new, sample_2, 0);
	printf("%slinkat(%d%s, \"%s\"%s, %d%s, \"%s\"%s, 0) = %s\n",
	       my_secontext,
	       dfd_old, dfd_old_secontext,
	       sample_1, sample_1_secontext,
	       dfd_new, sample_2_dir_secontext,
	       sample_2, SELINUX_FILECONTEXT(new_sample_2),
	       sprintrc(rc));

	char *new_sample_2_realpath = xasprintf("%s/%s", sample_2_dir_realpath, new_sample_2);

	/* dfd ignored when path is absolute */
	if (chdir("..") == -1)
		perror_msg_and_fail("chdir");

	rc = syscall(__NR_linkat, dfd_old, sample_1, -100, new_sample_2_realpath, 0);
	printf("%slinkat(%d%s, \"%s\"%s, AT_FDCWD, \"%s\"%s, 0) = %s\n",
	       my_secontext,
	       dfd_old, dfd_old_secontext,
	       sample_1, sample_1_secontext,
	       new_sample_2_realpath, SELINUX_FILECONTEXT(new_sample_2_realpath),
	       sprintrc(rc));

	if (fchdir(dfd_old) == -1)
		perror_msg_and_fail("fchdir");

	unlink(sample_1);
	unlink(sample_2);
	unlink(new_sample_2);
	rmdir(sample_2_dir);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_linkat")

#endif
