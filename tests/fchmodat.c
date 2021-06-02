/*
 * Check decoding of fchmodat syscall.
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
#include <stdio.h>
#include <unistd.h>

#include "secontext.h"

int
main(void)
{
	/*
	 * Make sure the current workdir of the tracee
	 * is different from the current workdir of the tracer.
	 */
	create_and_enter_subdir("fchmodat_subdir");

	char *my_secontext = SECONTEXT_PID_MY();

	static const char sample[] = "fchmodat_sample_file";
	if (open(sample, O_RDONLY | O_CREAT, 0400) < 0)
		perror_msg_and_fail("open");

	char *sample_secontext = SECONTEXT_FILE(sample);

	/*
	 * Tests with AT_FDCWD.
	 */

	long rc = syscall(__NR_fchmodat, -100, sample, 0600);
	printf("%s%s(AT_FDCWD, \"%s\"%s, 0600) = %s\n",
	       my_secontext, "fchmodat",
	       sample, sample_secontext,
	       sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_fchmodat, -100, sample, 051);
	printf("%s%s(AT_FDCWD, \"%s\", 051) = %s\n",
	       my_secontext, "fchmodat",
	       sample, sprintrc(rc));

	rc = syscall(__NR_fchmodat, -100, sample, 004);
	printf("%s%s(AT_FDCWD, \"%s\", 004) = %s\n",
	       my_secontext, "fchmodat",
	       sample, sprintrc(rc));

	/*
	 * Tests with dirfd.
	 */

	int cwd_fd = get_dir_fd(".");
	char *cwd = get_fd_path(cwd_fd);
	char *cwd_secontext = SECONTEXT_FILE(".");
	char *sample_realpath = xasprintf("%s/%s", cwd, sample);

	/* no file */
	rc = syscall(__NR_fchmodat, cwd_fd, sample, 0400);
	printf("%s%s(%d%s, \"%s\", 0400) = %s\n",
	       my_secontext, "fchmodat",
	       cwd_fd, cwd_secontext,
	       sample,
	       sprintrc(rc));

	if (open(sample, O_RDONLY | O_CREAT, 0400) < 0)
		perror_msg_and_fail("open");

	rc = syscall(__NR_fchmodat, cwd_fd, sample, 0400);
	printf("%s%s(%d%s, \"%s\"%s, 0400) = %s\n",
	       my_secontext, "fchmodat",
	       cwd_fd, cwd_secontext,
	       sample, sample_secontext,
	       sprintrc(rc));

	/* cwd_fd ignored when path is absolute */
	if (chdir("../.."))
		perror_msg_and_fail("chdir");

	rc = syscall(__NR_fchmodat, cwd_fd, sample_realpath, 0400);
	printf("%s%s(%d%s, \"%s\"%s, 0400) = %s\n",
	       my_secontext, "fchmodat",
	       cwd_fd, cwd_secontext,
	       sample_realpath, sample_secontext,
	       sprintrc(rc));

	if (fchdir(cwd_fd))
		perror_msg_and_fail("fchdir");

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	leave_and_remove_subdir();

	puts("+++ exited with 0 +++");
	return 0;
}
