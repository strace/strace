/*
 * Check printing of the counter endpoint of /dev/ptmx in -yy mode.
 *
 * Copyright (c) 2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int
main(void)
{
	static const char dev_ptmx[] = "/dev/ptmx";
	int ptmx_fd = open(dev_ptmx, O_RDWR|O_NOCTTY);
	if (ptmx_fd < 0)
		perror_msg_and_skip("open: %s", dev_ptmx);

	if (unlockpt(ptmx_fd) < 0)
		perror_msg_and_skip("unlockpt");

	const char *pts = ptsname(ptmx_fd);
	if (pts == NULL)
		error_msg_and_skip("failed in ptsname()");

	const char *real_ptmx = get_fd_path(ptmx_fd);
	int rc = close(ptmx_fd);
	printf("close(%d<%s<char 5:2 @%s>>) = %d\n",
	       ptmx_fd, real_ptmx, pts, rc);

	puts("+++ exited with 0 +++");
	return 0;
}
