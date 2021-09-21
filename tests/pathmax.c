/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * This test is designed to cover cases where decoding cannot happen because
 * paths length exceed PATH_MAX.
 * It is designed to execute with -y or similar option.
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
	 * AT_FDCWD decoding
	 */

	char name[NAME_MAX + 1];
	unsigned int i;
	for (i = 0; i < sizeof(name)-1; i++)
		name[i] = 'x';
	name[sizeof(name)-1] = '\0';

	unsigned long cwdlen = strlen(topdir);
	unsigned int count = 0;

	for (cwdlen = strlen(topdir);
	     cwdlen <= PATH_MAX;
	     cwdlen += strlen(name) + 1, count++) {
		if (mkdir(name, 0700) == -1)
			perror_msg_and_fail("mkdir");
		if (chdir(name) == -1)
			perror_msg_and_fail("chdir");
	}

	/* AT_FDCWD is not be printed since path cannot be resolved */

	int fd = openat(AT_FDCWD, "sample", O_RDONLY);
	printf("openat(AT_FDCWD, \"sample\", O_RDONLY) = %s\n",
	       sprintrc(fd));

	/* Go back one dir and verify it's printed */

	count--;
	if (chdir("..") == -1)
		perror_msg_and_fail("chdir");
	if (rmdir(name) == -1)
		perror_msg_and_fail("rmdir");

	char *cwd = get_fd_path(get_dir_fd("."));

	fd = openat(AT_FDCWD, "sample", O_RDONLY);
	printf("openat(AT_FDCWD<%s>, \"sample\", O_RDONLY) = %s\n",
	       cwd, sprintrc(fd));

	/* Create a dir for which exact PATH_MAX size is returned */

	char dir[NAME_MAX + 1];
	for (i = 0; i < sizeof(dir)-1; i++)
		dir[i] = 'x';
	dir[PATH_MAX - (strlen(cwd) + 1)] = '\0';
	if (mkdir(dir, 0700) == -1)
		perror_msg_and_fail("mkdir");
	if (chdir(dir) == -1)
		perror_msg_and_fail("chdir");

	/* AT_FDCWD is not preinted since path cannot be resolved fully */

	fd = openat(AT_FDCWD, "sample", O_RDONLY);
	printf("openat(AT_FDCWD, \"sample\", O_RDONLY) = %s\n",
	       sprintrc(fd));

	if (chdir("..") == -1)
		perror_msg_and_fail("chdir");
	if (rmdir(dir) == -1)
		perror_msg_and_fail("rmdir");

	for (i = count; i > 0; i--) {
		if (chdir("..") == -1)
			perror_msg_and_fail("chdir");
		if (rmdir(name) == -1)
			perror_msg_and_fail("rmdir");
	}

	leave_and_remove_subdir();

	puts("+++ exited with 0 +++");
	return 0;
}
