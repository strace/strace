/*
 * Check decoding of fchownat syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <fcntl.h>

#if defined __NR_fchownat && defined AT_FDCWD && defined AT_SYMLINK_NOFOLLOW

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "selinux.c"

int
main(void)
{
	static const char sample[] = "fchownat_sample";
	char *my_secontext = SELINUX_MYCONTEXT();
	uid_t uid = geteuid();
	uid_t gid = getegid();
	int fd = open(sample, O_RDONLY | O_CREAT, 0400);

	if (fd == -1)
		perror_msg_and_fail("open");
	close(fd);

	char *sample_secontext = SELINUX_FILECONTEXT(sample);

	long rc = syscall(__NR_fchownat, AT_FDCWD, sample, uid, gid, 0);
	printf("%sfchownat(AT_FDCWD, \"%s\"%s, %d, %d, 0) = %s\n",
	       my_secontext,
	       sample, sample_secontext,
	       uid, gid, sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_fchownat, AT_FDCWD,
		     sample, -1, -1L, AT_SYMLINK_NOFOLLOW);
	printf("%sfchownat(AT_FDCWD, \"%s\", -1, -1, AT_SYMLINK_NOFOLLOW) = %s\n",
	       my_secontext,
	       sample, sprintrc(rc));

	/*
	 * Tests with dirfd
	 */

	char *cwd = NULL;
	int cwd_fd = get_curdir_fd(&cwd);
	char *cwd_secontext = SELINUX_FILECONTEXT(".");
	char *sample_realpath = xasprintf("%s/%s", cwd, sample);

	/* no file */
	rc = syscall(__NR_fchownat, cwd_fd, sample, uid, gid, 0);
	printf("%sfchownat(%d%s, \"%s\", %d, %d, 0) = %s\n",
	       my_secontext,
	       cwd_fd, cwd_secontext,
	       sample,
	       uid, gid,
	       sprintrc(rc));

	fd = open(sample, O_RDONLY | O_CREAT, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");
	close(fd);

	rc = syscall(__NR_fchownat, cwd_fd, sample, uid, gid, 0);
	printf("%sfchownat(%d%s, \"%s\"%s, %d, %d, 0) = %s\n",
	       my_secontext,
	       cwd_fd, cwd_secontext,
	       sample, sample_secontext,
	       uid, gid,
	       sprintrc(rc));

	/* cwd_fd ignored when path is absolute */
	if (chdir("..") == -1)
		perror_msg_and_fail("chdir");

	rc = syscall(__NR_fchownat, cwd_fd, sample_realpath, uid, gid, 0);
	printf("%sfchownat(%d%s, \"%s\"%s, %d, %d, 0) = %s\n",
	       my_secontext,
	       cwd_fd, cwd_secontext,
	       sample_realpath, sample_secontext,
	       uid, gid,
	       sprintrc(rc));

	if (fchdir(cwd_fd) == -1)
		perror_msg_and_fail("fchdir");

	unlink(sample);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fchownat && AT_FDCWD && AT_SYMLINK_NOFOLLOW")

#endif
