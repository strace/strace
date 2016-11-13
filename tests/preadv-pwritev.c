/*
 * Check decoding of preadv and pwritev syscalls.
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

#if defined HAVE_PREADV && defined HAVE_PWRITEV

# include <fcntl.h>
# include <stdio.h>
# include <sys/uio.h>
# include <unistd.h>

int
main(void)
{
	tprintf("%s", "");

	static char tmp[] = "preadv-pwritev-tmpfile";
	if (open(tmp, O_CREAT|O_RDONLY|O_TRUNC, 0600) != 0)
		perror_msg_and_fail("creat: %s", tmp);
	if (open(tmp, O_WRONLY) != 1)
		perror_msg_and_fail("open: %s", tmp);
	if (unlink(tmp))
		perror_msg_and_fail("unlink: %s", tmp);

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

	rc = pwritev(1, efault, 42, 0);
	tprintf("pwritev(1, %p, 42, 0) = %ld %s (%m)\n",
		efault, rc, errno2name());

	rc = preadv(0, efault, 42, 0);
	tprintf("preadv(0, %p, 42, 0) = %ld %s (%m)\n",
		efault, rc, errno2name());

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

	rc = pwritev(1, w_iov, 0, 0);
	if (rc)
		perror_msg_and_fail("pwritev: expected 0, returned %ld", rc);
	tprintf("pwritev(1, [], 0, 0) = 0\n");

	rc = pwritev(1, w_iov + ARRAY_SIZE(w_iov_) - 1, 2, 0);
	tprintf("pwritev(1, [{iov_base=\"%s\", iov_len=%u}, %p], 2, 0)"
		" = %ld %s (%m)\n",
		w2_c, LENGTH_OF(w2_c), w_iov + ARRAY_SIZE(w_iov_),
		rc, errno2name());

	const unsigned int w_len =
		LENGTH_OF(w0_c) + LENGTH_OF(w1_c) + LENGTH_OF(w2_c);

	rc = pwritev(1, w_iov, ARRAY_SIZE(w_iov_), 0);
	if (rc != (int) w_len)
		perror_msg_and_fail("pwritev: expected %u, returned %ld",
				    w_len, rc);
	close(1);
	tprintf("pwritev(1, [{iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"%s\", iov_len=%u}], %u, 0) = %u\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n"
		" * %u bytes in buffer 1\n"
		" | 00000 %-49s  %-16s |\n"
		" * %u bytes in buffer 2\n"
		" | 00000 %-49s  %-16s |\n",
		w0_c, LENGTH_OF(w0_c), w1_c, LENGTH_OF(w1_c),
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

	rc = preadv(0, r_iov, ARRAY_SIZE(r0_iov_), 0);
	if (rc != (int) r_len)
		perror_msg_and_fail("preadv: expected %u, returned %ld",
				    r_len, rc);
	tprintf("preadv(0, [{iov_base=\"%s\", iov_len=%u}], %u, 0) = %u\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n",
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

	rc = preadv(0, r_iov, ARRAY_SIZE(r1_iov_), r_len);
	if (rc != (int) w_len - (int) r_len)
		perror_msg_and_fail("preadv: expected %d, returned %ld",
				    (int) w_len - r_len, rc);
	tprintf("preadv(0, [{iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"\", iov_len=%u}], %u, %u) = %u\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n",
		r1_c, r_len, w_len, ARRAY_SIZE(r1_iov_),
		r_len, w_len - r_len,
		w_len - r_len, r1_d, r1_c);
	close(0);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_PREADV && HAVE_PWRITEV")

#endif
