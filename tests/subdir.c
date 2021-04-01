/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

static const char *subdir;
static DIR *dirp;

void
create_and_enter_subdir(const char *name)
{
	dirp = opendir(".");
	if (!dirp)
		perror_msg_and_fail("opendir: %s", ".");
	(void) mkdir(name, 0700);
	if (chdir(name))
		perror_msg_and_fail("chdir: %s", name);
	subdir = name;
}

void
leave_and_remove_subdir(void)
{
	if (fchdir(dirfd(dirp)))
		perror_msg_and_fail("fchdir: %d", dirfd(dirp));
	if (closedir(dirp))
		perror_msg_and_fail("closedir");
	dirp = 0;
	if (rmdir(subdir))
		perror_msg_and_fail("rmdir: %s", subdir);
	subdir = 0;
}
