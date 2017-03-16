/*
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_PREADV

# include <fcntl.h>
# include <stdio.h>
# include <sys/uio.h>
# include <unistd.h>

# define LEN 8

static void
print_iov(const struct iovec *iov)
{
	unsigned int i;
	unsigned char *buf = iov->iov_base;

	fputs("{iov_base=\"", stdout);
	for (i = 0; i < iov->iov_len; ++i)
		printf("\\%d", (int) buf[i]);
	printf("\", iov_len=%u}", (unsigned) iov->iov_len);
}

static void
print_iovec(const struct iovec *iov, unsigned int cnt)
{
	unsigned int i;
	putchar('[');
	for (i = 0; i < cnt; ++i) {
		if (i)
			fputs(", ", stdout);
		print_iov(&iov[i]);
	}
	putchar(']');
}

int
main(void)
{
	const off_t offset = 0xdefaceddeadbeefLL;
	char *buf = tail_alloc(LEN);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct iovec, iov);
	iov->iov_base = buf;
	iov->iov_len = LEN;

	(void) close(0);
	if (open("/dev/zero", O_RDONLY))
		perror_msg_and_fail("open");

	if (preadv(0, iov, 1, offset) != LEN)
		perror_msg_and_fail("preadv");
	printf("preadv(0, ");
	print_iovec(iov, 1);
	printf(", 1, %lld) = %u\n", (long long) offset, LEN);

	if (preadv(0, iov, 1, -1) != -1)
		perror_msg_and_fail("preadv");
	printf("preadv(0, [{iov_base=%p, iov_len=%zu}], 1, -1) = "
	       "-1 EINVAL (%m)\n", iov->iov_base, iov->iov_len);

	if (preadv(0, NULL, 1, -2) != -1)
		perror_msg_and_fail("preadv");
	printf("preadv(0, NULL, 1, -2) = -1 EINVAL (%m)\n");

	if (preadv(0, iov, 0, -3) != -1)
		perror_msg_and_fail("preadv");
	printf("preadv(0, [], 0, -3) = -1 EINVAL (%m)\n");

	static const char tmp[] = "preadv-tmpfile";
	int fd = open(tmp, O_RDWR | O_CREAT | O_TRUNC, 0600);
	if (fd < 0)
		perror_msg_and_fail("open");
	if (unlink(tmp))
		perror_msg_and_fail("unlink");

	static const char w[] = "0123456789abcde";
	if (write(fd, w, LENGTH_OF(w)) != LENGTH_OF(w))
		perror_msg_and_fail("write");

	static const char r0_c[] = "01234567";
	static const char r1_c[] = "89abcde";

	const unsigned int r_len = (LENGTH_OF(w) + 1) / 2;
	void *r0 = tail_alloc(r_len);
	const struct iovec r0_iov_[] = {
		{
			.iov_base = r0,
			.iov_len = r_len
		}
	};
	const struct iovec *r_iov = tail_memdup(r0_iov_, sizeof(r0_iov_));

	long rc;

	rc = preadv(fd, r_iov, ARRAY_SIZE(r0_iov_), 0);
	if (rc != (int) r_len)
		perror_msg_and_fail("preadv: expected %u, returned %ld",
				    r_len, rc);
	printf("preadv(%d, [{iov_base=\"%s\", iov_len=%u}], %u, 0) = %u\n",
	       fd, r0_c, r_len, ARRAY_SIZE(r0_iov_), r_len);

	void *r1 = tail_alloc(r_len);
	void *r2 = tail_alloc(LENGTH_OF(w));
	const struct iovec r1_iov_[] = {
		{
			.iov_base = r1,
			.iov_len = r_len
		},
		{
			.iov_base = r2,
			.iov_len = LENGTH_OF(w)
		}
	};
	r_iov = tail_memdup(r1_iov_, sizeof(r1_iov_));

	rc = preadv(fd, r_iov, ARRAY_SIZE(r1_iov_), r_len);
	if (rc != (int) LENGTH_OF(w) - (int) r_len)
		perror_msg_and_fail("preadv: expected %d, returned %ld",
				    (int) LENGTH_OF(w) - r_len, rc);
	printf("preadv(%d, [{iov_base=\"%s\", iov_len=%u}"
	       ", {iov_base=\"\", iov_len=%u}], %u, %u) = %u\n",
	       fd, r1_c, r_len, LENGTH_OF(w), ARRAY_SIZE(r1_iov_),
		r_len, LENGTH_OF(w) - r_len);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_PREADV")

#endif
