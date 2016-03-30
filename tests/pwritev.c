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

#ifdef HAVE_PWRITEV

# include <fcntl.h>
# include <stdio.h>
# include <sys/uio.h>
# include <unistd.h>

# define LEN 8
# define LIM (LEN - 1)

static void
print_iov(const struct iovec *iov)
{
	unsigned int i;
	unsigned char *buf = iov->iov_base;

	fputs("{\"", stdout);
	for (i = 0; i < iov->iov_len; ++i) {
		if (i < LIM)
			printf("\\%d", (int) buf[i]);
	}
	printf("\"%s, %u}",
	       i > LIM ? "..." : "", (unsigned) iov->iov_len);
}

static void
print_iovec(const struct iovec *iov, unsigned int cnt, unsigned int size)
{
	if (!size) {
		printf("%p", iov);
		return;
	}
	unsigned int i;
	putchar('[');
	for (i = 0; i < cnt; ++i) {
		if (i)
			fputs(", ", stdout);
		if (i == LIM) {
			fputs("...", stdout);
			break;
		}
		if (i == size) {
			printf("%p", &iov[i]);
			break;
		}
		print_iov(&iov[i]);
	}
	putchar(']');
}

int
main(void)
{
	(void) close(0);
	if (open("/dev/null", O_WRONLY))
		perror_msg_and_fail("open");

	char *buf = tail_alloc(LEN);
	unsigned i;
	for (i = 0; i < LEN; ++i)
		buf[i] = i;

	struct iovec *iov = tail_alloc(sizeof(*iov) * LEN);
	for (i = 0; i < LEN; ++i) {
		buf[i] = i;
		iov[i].iov_base = &buf[i];
		iov[i].iov_len = LEN - i;
	}
	tail_alloc(1);

	const off_t offset = 0xdefaceddeadbeefLL;
	int written = 0;
	for (i = 0; i < LEN; ++i) {
		written += iov[i].iov_len;
		if (pwritev(0, iov, i + 1, offset + i) != written)
			perror_msg_and_fail("pwritev");
		fputs("pwritev(0, ", stdout);
		print_iovec(iov, i + 1, LEN);
		printf(", %u, %lld) = %d\n",
		       i + 1, (long long) offset + i, written);
	}

	for (i = 0; i <= LEN; ++i) {
		unsigned int n = LEN + 1 - i;
		if (pwritev(0, iov + i, n, offset + LEN + i) != -1)
			perror_msg_and_fail("pwritev");
		fputs("pwritev(0, ", stdout);
		print_iovec(iov + i, n, LEN - i);
		printf(", %u, %lld) = -1 EFAULT (%m)\n",
		       n, (long long) offset + LEN + i);
	}

	iov->iov_base = iov + LEN * 2;
	if (pwritev(0, iov, 1, -1) != -1)
		perror_msg_and_fail("pwritev");
	printf("pwritev(0, [{%p, %d}], 1, -1) = -1 EINVAL (%m)\n",
	       iov->iov_base, LEN);

	iov += LEN;
	if (pwritev(0, iov, 42, -2) != -1)
		perror_msg_and_fail("pwritev");
	printf("pwritev(0, %p, 42, -2) = -1 EINVAL (%m)\n", iov);

	if (pwritev(0, NULL, 1, -3) != -1)
		perror_msg_and_fail("pwritev");
	printf("pwritev(0, NULL, 1, -3) = -1 EINVAL (%m)\n");

	if (pwritev(0, NULL, 0, -4) != -1)
		perror_msg_and_fail("pwritev");
	printf("pwritev(0, [], 0, -4) = -1 EINVAL (%m)\n");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_PWRITEV")

#endif
