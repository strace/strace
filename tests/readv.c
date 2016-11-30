/*
 * Check decoding of readv and writev syscalls.
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
#include <stdio.h>
#include <unistd.h>
#include <sys/uio.h>

int
main(void)
{
	tprintf("%s", "");

	int fds[2];
	pipe_maxfd(fds);

	static const char w0_c[] = "012";
	const char *w0_d = hexdump_strdup(w0_c);
	void *w0 = tail_memdup(w0_c, LENGTH_OF(w0_c));

	const void *efault = w0 + LENGTH_OF(w0_c);

	static const char w1_c[] = "34567";
	const char *w1_d = hexdump_strdup(w1_c);
	void *w1 = tail_memdup(w1_c, LENGTH_OF(w1_c));

	static const char w2_c[] = "89abcde";
	const char *w2_d = hexdump_strdup(w2_c);
	void *w2 = tail_memdup(w2_c, LENGTH_OF(w2_c));
	long rc;

	rc = writev(fds[1], efault, 42);
	tprintf("writev(%d, %p, 42) = %ld %s (%m)\n",
		fds[1], efault, rc, errno2name());

	rc = readv(fds[0], efault, 42);
	tprintf("readv(%d, %p, 42) = %ld %s (%m)\n",
		fds[0], efault, rc, errno2name());

	static const char r0_c[] = "01234567";
	const char *r0_d = hexdump_strdup(r0_c);
	static const char r1_c[] = "89abcde";
	const char *r1_d = hexdump_strdup(r1_c);

	const struct iovec w_iov_[] = {
		{
			.iov_base = w0,
			.iov_len = LENGTH_OF(w0_c)
		}, {
			.iov_base = w1,
			.iov_len = LENGTH_OF(w1_c)
		}, {
			.iov_base = w2,
			.iov_len = LENGTH_OF(w2_c)
		}
	};
	const struct iovec *w_iov = tail_memdup(w_iov_, sizeof(w_iov_));

	tprintf("writev(%d, [], 0) = %ld\n",
		fds[1], (long) writev(fds[1], w_iov, 0));

	rc = writev(fds[1], w_iov + ARRAY_SIZE(w_iov_) - 1, 2);
	tprintf("writev(%d, [{iov_base=\"%s\", iov_len=%u}, %p], 2)"
		" = %ld %s (%m)\n",
		fds[1], w2_c, LENGTH_OF(w2_c), w_iov + ARRAY_SIZE(w_iov_),
		rc, errno2name());

	const unsigned int w_len =
		LENGTH_OF(w0_c) + LENGTH_OF(w1_c) + LENGTH_OF(w2_c);

	assert(writev(fds[1], w_iov, ARRAY_SIZE(w_iov_)) == (int) w_len);
	close(fds[1]);
	tprintf("writev(%d, [{iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"%s\", iov_len=%u}], %u) = %u\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n"
		" * %u bytes in buffer 1\n"
		" | 00000 %-49s  %-16s |\n"
		" * %u bytes in buffer 2\n"
		" | 00000 %-49s  %-16s |\n",
		fds[1], w0_c, LENGTH_OF(w0_c), w1_c, LENGTH_OF(w1_c),
		w2_c, LENGTH_OF(w2_c), ARRAY_SIZE(w_iov_), w_len,
		LENGTH_OF(w0_c), w0_d, w0_c,
		LENGTH_OF(w1_c), w1_d, w1_c, LENGTH_OF(w2_c), w2_d, w2_c);

	const unsigned int r_len = (w_len + 1) / 2;
	void *r0 = tail_alloc(r_len);
	const struct iovec r0_iov_[] = {
		{
			.iov_base = r0,
			.iov_len = r_len
		}
	};
	const struct iovec *r_iov = tail_memdup(r0_iov_, sizeof(r0_iov_));

	assert(readv(fds[0], r_iov, ARRAY_SIZE(r0_iov_)) == (int) r_len);
	tprintf("readv(%d, [{iov_base=\"%s\", iov_len=%u}], %u) = %u\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n",
		fds[0],
		r0_c, r_len, ARRAY_SIZE(r0_iov_), r_len, r_len, r0_d, r0_c);

	void *r1 = tail_alloc(r_len);
	void *r2 = tail_alloc(w_len);
	const struct iovec r1_iov_[] = {
		{
			.iov_base = r1,
			.iov_len = r_len
		},
		{
			.iov_base = r2,
			.iov_len = w_len
		}
	};
	r_iov = tail_memdup(r1_iov_, sizeof(r1_iov_));

	assert(readv(fds[0], r_iov, ARRAY_SIZE(r1_iov_)) == (int) w_len - (int) r_len);
	tprintf("readv(%d, [{iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"\", iov_len=%u}], %u) = %u\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n",
		fds[0], r1_c, r_len, w_len, ARRAY_SIZE(r1_iov_), w_len - r_len,
		w_len - r_len, r1_d, r1_c);
	close(fds[0]);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
