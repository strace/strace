/*
 * Check decoding of fchmodat2 syscall.
 *
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "secontext.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#ifndef AT_SYMLINK_NOFOLLOW
# define AT_SYMLINK_NOFOLLOW	0x100
#endif
#ifndef AT_EMPTY_PATH
# define AT_EMPTY_PATH		0x1000
#endif

static const char *errstr;

static long
k_fchmodat2(const unsigned int dfd, const void *path,
	    const unsigned short mode, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xbadc0ded00000000ULL;
	const kernel_ulong_t fill_short = fill | 0xdead0000ULL;
	const kernel_ulong_t arg1 = fill | dfd;
	const kernel_ulong_t arg2 = (uintptr_t) path;
	const kernel_ulong_t arg3 = fill_short | mode;
	const kernel_ulong_t arg4 = fill | flags;
	const kernel_ulong_t arg5 = fill | 0xdecaffed;
	const kernel_ulong_t arg6 = fill | 0xdeefaced;
	const long rc = syscall(__NR_fchmodat2,
				arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	/*
	 * Make sure the current workdir of the tracee
	 * is different from the current workdir of the tracer.
	 */
	create_and_enter_subdir("fchmodat2_subdir");

	char *my_secontext = SECONTEXT_PID_MY();

	static const char sample[] = "fchmodat2_sample_file";
	if (open(sample, O_RDONLY | O_CREAT, 0400) < 0)
		perror_msg_and_fail("open");

	char *sample_secontext = SECONTEXT_FILE(sample);

	/*
	 * Tests with AT_FDCWD.
	 */

	k_fchmodat2(-100, sample, 0600, 0);
	printf("%s%s(%s, \"%s\"%s, 0600, 0) = %s\n",
	       my_secontext, "fchmodat2", "AT_FDCWD",
	       sample, sample_secontext, errstr);

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	k_fchmodat2(-100, sample, 051, AT_SYMLINK_NOFOLLOW);
	printf("%s%s(%s, \"%s\", 051, AT_SYMLINK_NOFOLLOW) = %s\n",
	       my_secontext, "fchmodat2", "AT_FDCWD", sample, errstr);

	k_fchmodat2(-100, sample, 0700, AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH);
	printf("%s%s(%s, \"%s\", 0700, AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH)"
	       " = %s\n",
	       my_secontext, "fchmodat2", "AT_FDCWD", sample, errstr);

	k_fchmodat2(-100, sample, 004,
		    -1U & ~(AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH));
	printf("%s%s(%s, \"%s\", 004, 0xffffeeff /* AT_??? */) = %s\n",
	       my_secontext, "fchmodat2", "AT_FDCWD", sample, errstr);

	/*
	 * Tests with dirfd.
	 */

	int cwd_fd = get_dir_fd(".");
	char *cwd = get_fd_path(cwd_fd);
	char *cwd_secontext = SECONTEXT_FILE(".");
	char *sample_realpath = xasprintf("%s/%s", cwd, sample);

	/* no file */
	k_fchmodat2(cwd_fd, sample, 0400, 0);
	printf("%s%s(%d%s, \"%s\", 0400, 0) = %s\n",
	       my_secontext, "fchmodat2",
	       cwd_fd, cwd_secontext, sample, errstr);

	if (open(sample, O_RDONLY | O_CREAT, 0400) < 0)
		perror_msg_and_fail("open");

	k_fchmodat2(cwd_fd, sample, 0400, AT_SYMLINK_NOFOLLOW);
	printf("%s%s(%d%s, \"%s\"%s, 0400, AT_SYMLINK_NOFOLLOW) = %s\n",
	       my_secontext, "fchmodat2",
	       cwd_fd, cwd_secontext, sample, sample_secontext, errstr);

	/* cwd_fd ignored when path is absolute */
	if (chdir("../.."))
		perror_msg_and_fail("chdir");

	k_fchmodat2(cwd_fd, sample_realpath, 0700, AT_EMPTY_PATH);
	printf("%s%s(%d%s, \"%s\"%s, 0700, AT_EMPTY_PATH) = %s\n",
	       my_secontext, "fchmodat2",
	       cwd_fd, cwd_secontext,
	       sample_realpath, sample_secontext, errstr);

	if (fchdir(cwd_fd))
		perror_msg_and_fail("fchdir");

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	leave_and_remove_subdir();

	puts("+++ exited with 0 +++");
	return 0;
}
