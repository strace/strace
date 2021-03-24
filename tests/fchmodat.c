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

#ifdef __NR_fchmodat

#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "selinux.c"

int
main(void)
{
	static const char sample[] = "fchmodat_sample";
	char *my_secontext = SELINUX_MYCONTEXT();

	if (open(sample, O_RDONLY | O_CREAT, 0400) < 0)
		perror_msg_and_fail("open");

	char *sample_secontext = SELINUX_FILECONTEXT(sample);

	long rc = syscall(__NR_fchmodat, -100, sample, 0600);
	printf("%sfchmodat(AT_FDCWD, \"%s\"%s, 0600) = %s\n",
	       my_secontext,
	       sample, sample_secontext,
	       sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_fchmodat, -100, sample, 051);
	printf("%sfchmodat(AT_FDCWD, \"%s\", 051) = %s\n",
	       my_secontext,
	       sample, sprintrc(rc));

	rc = syscall(__NR_fchmodat, -100, sample, 004);
	printf("%sfchmodat(AT_FDCWD, \"%s\", 004) = %s\n",
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
	rc = syscall(__NR_fchmodat, cwd_fd, sample, 0400);
	printf("%sfchmodat(%d%s, \"%s\", 0400) = %s\n",
	       my_secontext,
	       cwd_fd, cwd_secontext,
	       sample,
	       sprintrc(rc));

	if (open(sample, O_RDONLY | O_CREAT, 0400) < 0)
		perror_msg_and_fail("open");

	rc = syscall(__NR_fchmodat, cwd_fd, sample, 0400);
	printf("%sfchmodat(%d%s, \"%s\"%s, 0400) = %s\n",
	       my_secontext,
	       cwd_fd, cwd_secontext,
	       sample, sample_secontext,
	       sprintrc(rc));

	/* cwd_fd ignored when path is absolute */
	if (chdir("..") == -1)
		perror_msg_and_fail("chdir");

	rc = syscall(__NR_fchmodat, cwd_fd, sample_realpath, 0400);
	printf("%sfchmodat(%d%s, \"%s\"%s, 0400) = %s\n",
	       my_secontext,
	       cwd_fd, cwd_secontext,
	       sample_realpath, sample_secontext,
	       sprintrc(rc));

	if (fchdir(cwd_fd) == -1)
		perror_msg_and_fail("fchdir");

	unlink(sample);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fchmodat")

#endif
