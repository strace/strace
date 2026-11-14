/*
 * Check decoding of syncfs syscall.
 *
 * Copyright (c) 2016-2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#ifndef DECODE_FDS
# define DECODE_FDS 0
#endif

#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	const long invalid_fd = (long) 0xdeadbeefffffffffULL;
	long rc = syscall(__NR_syncfs, invalid_fd);

#ifndef PATH_TRACING
	printf("syncfs(%d) = %s\n", (int) invalid_fd, sprintrc(rc));
#endif

	static const char path[] = "/dev/full";
	int fd_path = open(path, O_WRONLY);
	if (fd_path < 0)
		perror_msg_and_fail("open: %s", path);

	char fd_str[64];
	snprintf(fd_str, sizeof(fd_str), "%d%s%s%s", fd_path,
				 DECODE_FDS ? "<" : "",
				 DECODE_FDS ? path : "",
				 DECODE_FDS ? ">" : "");

	rc = syscall(__NR_syncfs, fd_path);
	printf("syncfs(%s) = %s\n", fd_str, sprintrc(rc));

	close(fd_path);
	puts("+++ exited with 0 +++");
	return 0;
}
