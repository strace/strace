/*
 * Check corner cases of AT_FDCWD path decoding.
 *
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
 * This test is designed to cover cases where AT_FDCWD path decoding
 * cannot happen because paths length exceed PATH_MAX.
 * It should be executed with -y or a similar option.
 */

int main(void)
{
	/*
	 * Make sure the current workdir of the tracee
	 * is different from the current workdir of the tracer.
	 */
	create_and_enter_subdir("pathmax_subdir");

	char *topdir = get_fd_path(get_dir_fd("."));

	/*
	 * AT_FDCWD path decoding
	 */
	char name[NAME_MAX + 1];
	memset(name, 'x', sizeof(name) - 1);
	name[sizeof(name) - 1] = '\0';

	unsigned int count = 0;
	for (size_t len = strlen(topdir);
	     len <= PATH_MAX;
	     len += sizeof(name), ++count) {
		if (mkdir(name, 0700))
			perror_msg_and_fail("mkdir, count=%u", count);
		if (chdir(name))
			perror_msg_and_fail("chdir, count=%u", count);
	}

	/* AT_FDCWD is not be printed since path cannot be resolved.  */

	int fd = syscall(__NR_openat, AT_FDCWD, "sample", O_RDONLY);
	printf("openat(AT_FDCWD, \"sample\", O_RDONLY) = %s\n",
	       sprintrc(fd));

	/* Go back one dir and verify it's printed.  */

	--count;
	if (chdir(".."))
		perror_msg_and_fail("chdir");
	if (rmdir(name))
		perror_msg_and_fail("rmdir");

	char *cwd = get_fd_path(get_dir_fd("."));

	fd = syscall(__NR_openat, AT_FDCWD, "sample", O_RDONLY);
	printf("openat(AT_FDCWD<%s>, \"sample\", O_RDONLY) = %s\n",
	       cwd, sprintrc(fd));

	/* Create a dir for which exact PATH_MAX size is returned.  */

	char dir[NAME_MAX + 1];
	memset(dir, 'x', sizeof(dir) - 1);
	dir[PATH_MAX - (strlen(cwd) + 1)] = '\0';
	if (mkdir(dir, 0700))
		perror_msg_and_fail("mkdir");
	if (chdir(dir))
		perror_msg_and_fail("chdir");

	/* AT_FDCWD is not printed since path cannot be resolved fully.  */

	fd = syscall(__NR_openat, AT_FDCWD, "sample", O_RDONLY);
	printf("openat(AT_FDCWD, \"sample\", O_RDONLY) = %s\n",
	       sprintrc(fd));

	if (chdir(".."))
		perror_msg_and_fail("chdir");
	if (rmdir(dir))
		perror_msg_and_fail("rmdir");

	for (; count > 0; --count) {
		if (chdir(".."))
			perror_msg_and_fail("chdir, count=%u", count);
		if (rmdir(name))
			perror_msg_and_fail("rmdir, count=%u", count);
	}

	leave_and_remove_subdir();

	puts("+++ exited with 0 +++");
	return 0;
}
