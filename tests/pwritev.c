/*
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_pwritev

# include <fcntl.h>
# include <stdio.h>
# include <sys/uio.h>
# include <unistd.h>

# define LEN 8
# define LIM (LEN - 1)

static long
k_pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
	return syscall(__NR_pwritev, fd, iov, iovcnt,
		       (kernel_ulong_t) offset,
		       (kernel_ulong_t) ((unsigned long long) offset >> 32));
}

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

int
main(void)
{
	(void) close(0);
	if (open("/dev/null", O_WRONLY))
		perror_msg_and_fail("open");

	char *buf = tail_alloc(LEN);
	for (unsigned int i = 0; i < LEN; ++i)
		buf[i] = i;

	TAIL_ALLOC_OBJECT_VAR_ARR(struct iovec, iov, LEN);
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
		if (k_pwritev(0, iov, i + 1, offset + i) != written)
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
		rc = k_pwritev(0, iov + i, n, offset + LEN + i);
		printf(", %u, %lld) = %s\n",
		       n, (long long) offset + LEN + i, sprintrc(rc));
	}

	iov->iov_base = iov + LEN * 2;
	rc = k_pwritev(0, iov, 1, -1);
	printf("pwritev(0, [{iov_base=%p, iov_len=%d}], 1, -1) = %s\n",
	       iov->iov_base, LEN, sprintrc(rc));

	iov += LEN;
	rc = k_pwritev(0, iov, 42, -2);
	printf("pwritev(0, %p, 42, -2) = %s\n",
	       iov, sprintrc(rc));

	rc = k_pwritev(0, NULL, 1, -3);
	printf("pwritev(0, NULL, 1, -3) = %s\n",
	       sprintrc(rc));

	rc = k_pwritev(0, iov, 0, -4);
	printf("pwritev(0, [], 0, -4) = %s\n",
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pwritev")

#endif
