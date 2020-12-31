/*
 * Create a temporary file in the current directory.
 *
 * Copyright (c) 2020 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <fcntl.h>
#include <unistd.h>

#ifndef O_TMPFILE
# if defined __hppa__
#  define O_TMPFILE	040000000
# elif defined __alpha__
#  define O_TMPFILE	0100000000
# elif defined __sparc__
#  define O_TMPFILE	0200000000
# else
#  define O_TMPFILE	020000000
# endif
#endif

int
create_tmpfile(unsigned int flags)
{
	int fd = open(".", O_TMPFILE | O_DIRECTORY | O_EXCL | flags, 0600);

	if (fd < 0) {
		/*
		 * Since every test is executed in a separate directory,
		 * there is no need to protect from race conditions.
		 */
		static const char fname[] = "create_tmpfile";

		fd = open(fname, O_CREAT | O_EXCL | flags, 0600);
		if (fd < 0)
			perror_msg_and_fail("open: %s", fname);
		if (unlink(fname))
			perror_msg_and_fail("unlink: %s", fname);
	}

	return fd;
}
