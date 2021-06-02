/*
 * Check decoding of preadv2 and pwritev2 syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/uio.h>
#include <unistd.h>

static int
pr(const int fd, const struct iovec *const vec,
   const unsigned long vlen, const unsigned long pos)
{
	return syscall(__NR_preadv2, fd, vec, vlen, pos, 0L, 0L);
}

static int
pw(const int fd, const struct iovec *const vec,
   const unsigned long vlen, const unsigned long pos)
{
	return syscall(__NR_pwritev2, fd, vec, vlen, pos, 0L, 0L);
}

static void
dumpio(void)
{
	static char tmp[] = "preadv2-pwritev2-tmpfile";
	if (open(tmp, O_CREAT|O_RDONLY|O_TRUNC, 0600) != 0)
		perror_msg_and_fail("creat: %s", tmp);
	if (open(tmp, O_WRONLY) != 1)
		perror_msg_and_fail("open: %s", tmp);
	if (unlink(tmp))
		perror_msg_and_fail("unlink: %s", tmp);

	static const char w0_c[] = "012";
	const char *w0_d = hexdump_strdup(w0_c);
	void *w0 = tail_memdup(w0_c, LENGTH_OF(w0_c));

	static const char w1_c[] = "34567";
	const char *w1_d = hexdump_strdup(w1_c);
	void *w1 = tail_memdup(w1_c, LENGTH_OF(w1_c));

	static const char w2_c[] = "89abcde";
	const char *w2_d = hexdump_strdup(w2_c);
	void *w2 = tail_memdup(w2_c, LENGTH_OF(w2_c));

	long rc;

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

	rc = pw(1, w_iov, 0, 0);
	if (rc)
		perror_msg_and_fail("pwritev2: expected 0, returned %ld", rc);
	tprintf("pwritev2(1, [], 0, 0, 0) = 0\n");

	rc = pw(1, w_iov + ARRAY_SIZE(w_iov_) - 1, 2, 0);
	tprintf("pwritev2(1, [{iov_base=\"%s\", iov_len=%u}, ... /* %p */], 2, 0, 0)"
		" = %ld %s (%m)\n",
		w2_c, LENGTH_OF(w2_c), w_iov + ARRAY_SIZE(w_iov_),
		rc, errno2name());

	const unsigned int w_len =
		LENGTH_OF(w0_c) + LENGTH_OF(w1_c) + LENGTH_OF(w2_c);

	rc = pw(1, w_iov, ARRAY_SIZE(w_iov_), 0);
	if (rc != (int) w_len)
		perror_msg_and_fail("pwritev2: expected %u, returned %ld",
				    w_len, rc);
	close(1);
	tprintf("pwritev2(1, [{iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"%s\", iov_len=%u}], %u, 0, 0) = %u\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n"
		" * %u bytes in buffer 1\n"
		" | 00000 %-49s  %-16s |\n"
		" * %u bytes in buffer 2\n"
		" | 00000 %-49s  %-16s |\n",
		w0_c, LENGTH_OF(w0_c), w1_c, LENGTH_OF(w1_c),
		w2_c, LENGTH_OF(w2_c), (unsigned int) ARRAY_SIZE(w_iov_), w_len,
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

	rc = pr(0, r_iov, ARRAY_SIZE(r0_iov_), 0);
	if (rc != (int) r_len)
		perror_msg_and_fail("preadv2: expected %u, returned %ld",
				    r_len, rc);
	tprintf("preadv2(0, [{iov_base=\"%s\", iov_len=%u}], %u, 0, 0) = %u\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n",
		r0_c, r_len, (unsigned int) ARRAY_SIZE(r0_iov_),
		r_len, r_len, r0_d, r0_c);

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

	rc = pr(0, r_iov, ARRAY_SIZE(r1_iov_), r_len);
	if (rc != (int) w_len - (int) r_len)
		perror_msg_and_fail("preadv2: expected %d, returned %ld",
				    (int) w_len - r_len, rc);
	tprintf("preadv2(0, [{iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"\", iov_len=%u}], %u, %u, 0) = %u\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n",
		r1_c, r_len, w_len, (unsigned int) ARRAY_SIZE(r1_iov_),
		r_len, w_len - r_len,
		w_len - r_len, r1_d, r1_c);
	close(0);
}

int
main(void)
{
	const kernel_ulong_t vlen = (kernel_ulong_t) 0xfac1fed2dad3bef4ULL;
	const unsigned long long pos = 0x7ac5fed6dad7bef8;
	const kernel_ulong_t pos_l = (kernel_ulong_t) pos;
	long rc;
	bool skip_dumpio_test = false;

	tprintf("%s", "");

#if defined __x86_64__ && defined __ILP32__
	/*
	 * x32 is the only architecture where preadv2 takes 5 arguments,
	 * see preadv64v2 in kernel sources.
	 */
	rc = syscall(__NR_preadv2, -1, NULL, vlen, pos_l, 1);
#else
	const kernel_ulong_t pos_h =
		(sizeof(pos_l) == sizeof(pos)) ?
		(kernel_ulong_t) 0xbadc0deddeadbeefULL :
		(kernel_ulong_t) (pos >> 32);
	rc = syscall(__NR_preadv2, -1, NULL, vlen, pos_l, pos_h, 1);
#endif
	if (rc != -1)
		error_msg_and_fail("preadv2: expected -1, returned %ld", rc);
	switch (errno) {
		case ENOSYS:
			skip_dumpio_test = true;
			break;
		case EBADF:
			break;
		default:
			perror_msg_and_fail("preadv2");
	}
	tprintf("preadv2(-1, NULL, %lu, %lld, RWF_HIPRI) = %s\n",
		(unsigned long) vlen, pos, sprintrc(rc));

#if defined __x86_64__ && defined __ILP32__
	/*
	 * x32 is the only architecture where pwritev2 takes 5 arguments,
	 * see pwritev64v2 in kernel sources.
	 */
	rc = syscall(__NR_pwritev2, -1, NULL, vlen, pos_l, 1);
#else
	rc = syscall(__NR_pwritev2, -1, NULL, vlen, pos_l, pos_h, 1);
#endif
	if (rc != -1)
		error_msg_and_fail("pwritev2: expected -1, returned %ld", rc);
	switch (errno) {
		case ENOSYS:
			skip_dumpio_test = true;
			break;
		case EBADF:
			break;
		default:
			perror_msg_and_fail("pwritev2");
	}
	tprintf("pwritev2(-1, NULL, %lu, %lld, RWF_HIPRI) = %s\n",
		(unsigned long) vlen, pos, sprintrc(rc));

	if (!skip_dumpio_test)
		dumpio();

	tprintf("%s\n", "+++ exited with 0 +++");
	return 0;
}
