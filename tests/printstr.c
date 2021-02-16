/*
 * Check decoding of non-NUL-terminated strings when len == -1.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>

int
main(void)
{
	char *const buf = tail_alloc(DEFAULT_STRLEN + 1);
	struct iovec io = {
		.iov_base = buf,
		.iov_len = -1
	};
	int rc;

	buf[0] = 0;

	tprintf("%s", "");

	memset(buf + 1, 'X', DEFAULT_STRLEN);
	buf[DEFAULT_STRLEN - 1] = 0;

	rc = writev(-1, &io, 1);
	tprintf("writev(-1, [{iov_base=\"\\0%*s\\0\"..., iov_len=%lu}], 1)"
		" = %s\n", DEFAULT_STRLEN - 2, buf + 1, -1UL, sprintrc(rc));

	buf[DEFAULT_STRLEN - 1] = 'X';
	buf[DEFAULT_STRLEN] = 0;

	rc = writev(-1, &io, 1);
	tprintf("writev(-1, [{iov_base=\"\\0%*s\"..., iov_len=%lu}], 1)"
		" = %s\n", DEFAULT_STRLEN - 1, buf + 1, -1UL, sprintrc(rc));

	++io.iov_base;
	rc = writev(-1, &io, 1);
	tprintf("writev(-1, [{iov_base=%p, iov_len=%lu}], 1) = %s\n",
		io.iov_base, -1UL, sprintrc(rc));

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
