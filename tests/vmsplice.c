/*
 * This file is part of vmsplice strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_vmsplice

# include <assert.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/uio.h>

int
main(void)
{
	tprintf("%s", "");

	int fds[2];
	if (pipe(fds))
		perror_msg_and_fail("pipe");
	assert(0 == fds[0]);
	assert(1 == fds[1]);

	static const char w0_c[] = "012";
	const char *w0_d = hexdump_strdup(w0_c);
	void *w0 = tail_memdup(w0_c, LENGTH_OF(w0_c));

	static const char w1_c[] = "34567";
	const char *w1_d = hexdump_strdup(w1_c);
	void *w1 = tail_memdup(w1_c, LENGTH_OF(w1_c));

	static const char w2_c[] = "89abcde";
	const char *w2_d = hexdump_strdup(w2_c);
	void *w2 = tail_memdup(w2_c, LENGTH_OF(w2_c));

	const struct iovec iov_[] = {
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
	const struct iovec *iov = tail_memdup(iov_, sizeof(iov_));
	const unsigned int len =
		LENGTH_OF(w0_c) + LENGTH_OF(w1_c) + LENGTH_OF(w2_c);

	tprintf("vmsplice(1, [{iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"%s\", iov_len=%u}], %u, %s) = %u\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n"
		" * %u bytes in buffer 1\n"
		" | 00000 %-49s  %-16s |\n"
		" * %u bytes in buffer 2\n"
		" | 00000 %-49s  %-16s |\n",
		w0_c, LENGTH_OF(w0_c), w1_c, LENGTH_OF(w1_c),
		w2_c, LENGTH_OF(w2_c), (unsigned int) ARRAY_SIZE(iov_),
		"SPLICE_F_NONBLOCK", len,
		LENGTH_OF(w0_c), w0_d, w0_c,
		LENGTH_OF(w1_c), w1_d, w1_c, LENGTH_OF(w2_c), w2_d, w2_c);

	const long rc = syscall(__NR_vmsplice, 1, iov, ARRAY_SIZE(iov_), 2);
	if (rc < 0)
		perror_msg_and_skip("vmsplice");
	assert(rc == (int) len);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_vmsplice")

#endif
