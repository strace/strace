/*
 * Close stdin, move stdout to a non-standard descriptor, and print.
 *
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

static ssize_t
write_retry(int fd, const void *buf, size_t count)
{
	ssize_t rc;

	do {
		errno = 0;
		rc = write(fd, buf, count);
	} while (rc == -1 && EINTR == errno);

	if (rc <= 0)
		perror_msg_and_fail("write");

	return rc;
}

static void
write_loop(int fd, const char *buf, size_t count)
{
	ssize_t offset = 0;

	while (count > 0) {
		ssize_t block = write_retry(fd, &buf[offset], count);

		offset += block;
		count -= (size_t) block;
	}
}

void
tprintf(const char *fmt, ...)
{
	static int initialized;
	if (!initialized) {
		assert(dup2(1, 3) == 3);
		assert(close(1) == 0);
		(void) close(0);
		initialized = 1;
	}

	va_list p;
	va_start(p, fmt);

	static char buf[65536];
	int len = vsnprintf(buf, sizeof(buf), fmt, p);
	if (len < 0)
		perror_msg_and_fail("vsnprintf");
	assert((unsigned) len < sizeof(buf));

	write_loop(3, buf, len);

	va_end(p);
}
