/*
 * Check effectiveness of umovestr memory caching.
 *
 * Copyright (c) 2019-2020 Dmitry V. Levin <ldv@altlinux.org>
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
	const size_t size = get_page_size() + (DEFAULT_STRLEN - 1);
	char *const buf = tail_alloc(size);
	fill_memory_ex(buf, DEFAULT_STRLEN * 2, 'a', 'z' - 'a' + 1);

	struct iovec *const io = tail_alloc(sizeof(*io) * DEFAULT_STRLEN);
	for (unsigned int i = 0; i < DEFAULT_STRLEN; ++i) {
		io[i].iov_base = buf + i;
		io[i].iov_len = DEFAULT_STRLEN;
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
