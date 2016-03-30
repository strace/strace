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

	fputs("{\"", stdout);
	for (i = 0; i < iov->iov_len; ++i)
		printf("\\%d", (int) buf[i]);
	printf("\", %u}", (unsigned) iov->iov_len);
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
	struct iovec *iov = tail_alloc(sizeof(*iov));
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
	printf("preadv(0, %p, 1, -1) = -1 EINVAL (%m)\n", iov);

	if (preadv(0, NULL, 1, -2) != -1)
		perror_msg_and_fail("preadv");
	printf("preadv(0, NULL, 1, -2) = -1 EINVAL (%m)\n");

	if (preadv(0, NULL, 0, -3) != -1)
		perror_msg_and_fail("preadv");
	printf("preadv(0, [], 0, -3) = -1 EINVAL (%m)\n");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_PREADV")

#endif
