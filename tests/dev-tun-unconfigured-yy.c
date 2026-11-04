/*
 * Check -yy output for an unconfigured /dev/net/tun descriptor.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

int
main(void)
{
	static const char dev_tun[] = "/dev/net/tun";
	int fd = open(dev_tun, O_RDWR);
	if (fd < 0)
		perror_msg_and_skip("open: %s", dev_tun);

	struct stat st;
	if (fstat(fd, &st) < 0)
		perror_msg_and_skip("fstat: %s", dev_tun);

	const char *path = get_fd_path(fd);
	int rc = close(fd);
	printf("close(%d<%s<char %u:%u>>) = %d\n",
	       fd, path, major(st.st_rdev), minor(st.st_rdev), rc);

	puts("+++ exited with 0 +++");
	return 0;
}
