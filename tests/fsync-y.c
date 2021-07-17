/*
 * Check printing of file name in strace -y mode.
 *
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	char dir[PATH_MAX + 1];

	const struct {
		const char *path;
		const char *cstr;
		const char *fdstr;
	} checks[] = {
		{ ARG_STR("\1\0020\v\0047\f\58\t\79\n\10\0171\r\0167\218\37 \\\'\"<<0::0>>1~\177\200\377"),
			"\\1\\0020\\v\\0047\\f\\58\\t\\79\\n\\10\\0171\\r\\0167"
			"\\218\\37 \\\\\'\\\"\\74\\0740::0\\76\\0761~\\177\\200\\377" },
	};

	if (!getcwd(dir, sizeof(dir)))
		perror_msg_and_fail("getcwd");

	for (unsigned int i = 0; i < ARRAY_SIZE(checks); i++) {
		long fd = open(checks[i].path, O_RDONLY|O_CREAT, 0600);
		if (fd < 0)
			perror_msg_and_fail("open(%s)", checks[i].cstr);

		int rc = fsync(fd);

		printf("fsync(%ld<", fd);
		print_quoted_string_ex(dir, false, ">:");
		printf("/%s>) = %s\n", checks[i].fdstr, sprintrc(rc));

		close(fd);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
