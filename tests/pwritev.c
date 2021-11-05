/*
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

	fputs("{iov_base=\"", stdout);
	for (i = 0; i < iov->iov_len; ++i) {
		if (i < LIM)
			printf("\\%d", (int) buf[i]);
	}
	printf("\"%s, iov_len=%u}",
	       i > LIM ? "..." : "", (unsigned) iov->iov_len);
}

static void
print_iovec(const struct iovec *iov, unsigned int cnt, unsigned int size)
{
	if (!size) {
		printf("%p", iov);
		return;
	}
	putchar('[');
	for (unsigned int i = 0; i < cnt; ++i) {
		if (i)
			fputs(", ", stdout);
		if (i == size) {
			printf("... /* %p */", &iov[i]);
			break;
		}
		if (i == LIM) {
			fputs("...", stdout);
			break;
		}
		print_iov(&iov[i]);
	}
	putchar(']');
}

/* for pwritev(0, NULL, 1, -3) */
DIAG_PUSH_IGNORE_NONNULL

int
main(void)
{
	(void) close(0);
	if (open("/dev/null", O_WRONLY))
		perror_msg_and_fail("open");

	char *buf = tail_alloc(LEN);
	for (unsigned int i = 0; i < LEN; ++i)
		buf[i] = i;

	struct iovec *iov = tail_alloc(sizeof(*iov) * LEN);
	for (unsigned int i = 0; i < LEN; ++i) {
		buf[i] = i;
		iov[i].iov_base = &buf[i];
		iov[i].iov_len = LEN - i;
	}

	const off_t offset = 0xdefaceddeadbeefLL;
	long rc;
	int written = 0;
	for (unsigned int i = 0; i < LEN; ++i) {
		written += iov[i].iov_len;
		if (pwritev(0, iov, i + 1, offset + i) != written)
			perror_msg_and_fail("pwritev");
		fputs("pwritev(0, ", stdout);
		print_iovec(iov, i + 1, LEN);
		printf(", %u, %lld) = %d\n",
		       i + 1, (long long) offset + i, written);
	}

	for (unsigned int i = 0; i <= LEN; ++i) {
		unsigned int n = LEN + 1 - i;
		fputs("pwritev(0, ", stdout);
		print_iovec(iov + i, n, LEN - i);
		rc = pwritev(0, iov + i, n, offset + LEN + i);
		printf(", %u, %lld) = %ld %s (%m)\n",
		       n, (long long) offset + LEN + i, rc, errno2name());
	}

	iov->iov_base = iov + LEN * 2;
	rc = pwritev(0, iov, 1, -1);
	printf("pwritev(0, [{iov_base=%p, iov_len=%d}], 1, -1) = %ld %s (%m)\n",
	       iov->iov_base, LEN, rc, errno2name());

	iov += LEN;
	rc = pwritev(0, iov, 42, -2);
	printf("pwritev(0, %p, 42, -2) = %ld %s (%m)\n",
	       iov, rc, errno2name());

	rc = pwritev(0, NULL, 1, -3);
	printf("pwritev(0, NULL, 1, -3) = %ld %s (%m)\n",
	       rc, errno2name());

	rc = pwritev(0, iov, 0, -4);
	printf("pwritev(0, [], 0, -4) = %ld %s (%m)\n",
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

DIAG_POP_IGNORE_NONNULL

#else

SKIP_MAIN_UNDEFINED("HAVE_PWRITEV")

#endif
