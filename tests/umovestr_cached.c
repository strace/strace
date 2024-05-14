/*
 * Check effectiveness of umovestr memory caching.
 *
 * Copyright (c) 2019-2024 Dmitry V. Levin <ldv@strace.io>
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
	char *const buf = tail_alloc(DEFAULT_STRLEN);
	fill_memory_ex(buf, DEFAULT_STRLEN, 'a', 'z' - 'a' + 1);

	TAIL_ALLOC_OBJECT_CONST_ARR(struct iovec, io, DEFAULT_STRLEN);
	for (unsigned int i = 0; i < DEFAULT_STRLEN; ++i) {
		io[i].iov_base = buf + DEFAULT_STRLEN - i;
		io[i].iov_len = i;
	}

	tprintf("%s", "");

	int rc = writev(-1, io, DEFAULT_STRLEN);
	const char *errstr = sprintrc(rc);

	tprintf("writev(-1, [");
	for (unsigned int i = 0; i < DEFAULT_STRLEN; ++i) {
		if (i)
			tprintf(", ");
		tprintf("{iov_base=\"%.*s\", iov_len=%u}",
			(int) io[i].iov_len,
			(char *) io[i].iov_base,
			(unsigned int) io[i].iov_len);
	}
	tprintf("], %u) = %s\n", DEFAULT_STRLEN, errstr);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
