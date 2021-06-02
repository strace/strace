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

#if defined AT_FDCWD && defined AT_SYMLINK_NOFOLLOW

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
	create_and_enter_subdir("fchownat_subdir");

	char *my_secontext = SECONTEXT_PID_MY();
	uid_t uid = geteuid();
	uid_t gid = getegid();

	static const char sample[] = "fchownat_sample";
	int fd = open(sample, O_RDONLY | O_CREAT, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");
	close(fd);

	char *sample_secontext = SECONTEXT_FILE(sample);

	/*
	 * Tests with AT_FDCWD.
	 */

	long rc = syscall(__NR_fchownat, AT_FDCWD, sample, uid, gid, 0);
	printf("%s%s(AT_FDCWD, \"%s\"%s, %d, %d, 0) = %s\n",
	       my_secontext, "fchownat",
	       sample, sample_secontext,
	       uid, gid, sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_fchownat, AT_FDCWD,
		     sample, -1, -1L, AT_SYMLINK_NOFOLLOW);
	printf("%s%s(AT_FDCWD, \"%s\", -1, -1, AT_SYMLINK_NOFOLLOW) = %s\n",
	       my_secontext, "fchownat",
	       sample, sprintrc(rc));

	/*
	 * Tests with dirfd.
	 */

	int cwd_fd = get_dir_fd(".");
	char *cwd = get_fd_path(cwd_fd);
	char *cwd_secontext = SECONTEXT_FILE(".");
	char *sample_realpath = xasprintf("%s/%s", cwd, sample);

	/* no file */
	rc = syscall(__NR_fchownat, cwd_fd, sample, uid, gid, 0);
	printf("%s%s(%d%s, \"%s\", %d, %d, 0) = %s\n",
	       my_secontext, "fchownat",
	       cwd_fd, cwd_secontext,
	       sample,
	       uid, gid,
	       sprintrc(rc));

	fd = open(sample, O_RDONLY | O_CREAT, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");
	close(fd);

	rc = syscall(__NR_fchownat, cwd_fd, sample, uid, gid, 0);
	printf("%s%s(%d%s, \"%s\"%s, %d, %d, 0) = %s\n",
	       my_secontext, "fchownat",
	       cwd_fd, cwd_secontext,
	       sample, sample_secontext,
	       uid, gid,
	       sprintrc(rc));

	/* cwd_fd ignored when path is absolute */
	if (chdir("../.."))
		perror_msg_and_fail("chdir");

	rc = syscall(__NR_fchownat, cwd_fd, sample_realpath, uid, gid, 0);
	printf("%s%s(%d%s, \"%s\"%s, %d, %d, 0) = %s\n",
	       my_secontext, "fchownat",
	       cwd_fd, cwd_secontext,
	       sample_realpath, sample_secontext,
	       uid, gid,
	       sprintrc(rc));

	if (fchdir(cwd_fd))
		perror_msg_and_fail("fchdir");

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	leave_and_remove_subdir();

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("AT_FDCWD && AT_SYMLINK_NOFOLLOW")

#endif
