/*
 * Check that fault injection works properly.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/uio.h>

static const int expfd = 4;
static const int gotfd = 5;

#define DEFAULT_ERRNO ENOSYS

static const char *errstr;
static int is_raw, err, first, step, iter, try;

static void
invoke(int fail)
{
	static char buf[sizeof(int) * 3 + 3];
	const struct iovec io = {
		.iov_base = buf,
		.iov_len = sprintf(buf, "%d.", ++try)
	};
	int rc;

	if (!fail) {
		rc = write(expfd, io.iov_base, io.iov_len);
		if (rc != (int) io.iov_len)
			perror_msg_and_fail("write");
	}

	errno = 0;
	rc = writev(gotfd, &io, 1);

	if (fail) {
		if (!(rc == -1 && errno == err))
			perror_msg_and_fail("expected errno %d"
					    ", got rc == %d, errno == %d",
					    err, rc, errno);

		if (is_raw)
			tprintf("writev(%#x, %p, 0x1) = -1 (errno %d)"
				" (INJECTED)\n", gotfd, &io, err);
		else
			tprintf("writev(%d, [{iov_base=\"%s\", iov_len=%d}], 1)"
				" = -1 %s (%m) (INJECTED)\n",
				gotfd, buf, (int) io.iov_len, errstr);
	} else {
		if (rc != (int) io.iov_len)
			perror_msg_and_fail("expected %d"
					    ", got rc == %d, errno == %d",
					    (int) io.iov_len, rc, errno);

		if (is_raw)
			tprintf("writev(%#x, %p, 0x1) = %#x\n", gotfd, &io, rc);
		else
			tprintf("writev(%d, [{iov_base=\"%s\", iov_len=%d}], 1)"
				" = %d\n",
				gotfd, buf, (int) io.iov_len, (int) io.iov_len);
	}
}

int
main(int argc, char *argv[])
{
	struct stat st;

	assert(fstat(expfd, &st) == 0);
	assert(fstat(gotfd, &st) == 0);

	assert(argc == 6);

	is_raw = !strcmp("raw", argv[1]);

	errstr = argv[2];
	err = atoi(errstr);
	assert(err >= 0);

	if (!err) {
		if (!*errstr)
			err = DEFAULT_ERRNO;
		else if (!strcasecmp(errstr, "EINVAL"))
			err = EINVAL;
		else
			err = ENOSYS;
	}

	errno = err;
	errstr = errno2name();

	first = atoi(argv[3]);
	step = atoi(argv[4]);
	iter = atoi(argv[5]);

	assert(first > 0);
	assert(step >= 0);

	tprintf("%s", "");

	int i;
	for (i = 1; i <= iter; ++i) {
		int fail = 0;
		if (first > 0) {
			--first;
			if (first == 0) {
				fail = 1;
				first = step;
			}
		}
		invoke(fail);
	}

	tprintf("%s\n", "+++ exited with 0 +++");
	return 0;
}
