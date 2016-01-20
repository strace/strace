/*
 * Close stdin, move stdout to a non-standard descriptor, and print.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
