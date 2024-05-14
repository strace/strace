/*
 * Check decoding of process_madvise syscall.
 *
 * Copyright (c) 2020-2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/uio.h>

#ifndef FD0_PATH
# define FD0_PATH ""
#endif

static const char *errstr;

static long
k_process_madvise(const unsigned int pidfd,
		  const struct iovec *const addr,
		  const kernel_ulong_t len,
		  const unsigned int cmd,
		  const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | pidfd;
	const kernel_ulong_t arg2 = (uintptr_t) addr;
	const kernel_ulong_t arg3 = len;
	const kernel_ulong_t arg4 = fill | cmd;
	const kernel_ulong_t arg5 = fill | flags;
	const long rc = syscall(__NR_process_madvise,
				arg1, arg2, arg3, arg4, arg5, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
#ifdef PRINT_PATHS
	skip_if_unavailable("/proc/self/fd/");
#endif

	(void) close(0);
	if (open("/dev/full", O_WRONLY))
		perror_msg_and_fail("open");

	TAIL_ALLOC_OBJECT_CONST_ARR(struct iovec, iov, 2);
	fill_memory(iov, 2 * sizeof(*iov));

	k_process_madvise(0, iov, 2, 0, -1U);
	printf("process_madvise(0" FD0_PATH
	       ", [{iov_base=%p, iov_len=%lu}, {iov_base=%p, iov_len=%lu}]"
	       ", 2, MADV_NORMAL, %#x) = %s\n",
	       iov[0].iov_base, (unsigned long) iov[0].iov_len,
	       iov[1].iov_base, (unsigned long) iov[1].iov_len,
	       -1U, errstr);

	const kernel_ulong_t bogus_len = (kernel_ulong_t) 0xdeadbeefdeadbeef;
	k_process_madvise(-1, 0, bogus_len, 20, 0);
	printf("process_madvise(-1, NULL, %" PRI_klu ", MADV_COLD, 0) = %s\n",
	       bogus_len, errstr);

	const unsigned int bogus_flags = 0xcafef00d;
	k_process_madvise(0, iov + 2, 0, 21, bogus_flags);
	printf("process_madvise(0" FD0_PATH ", [], 0, MADV_PAGEOUT, %#x)"
	       " = %s\n", bogus_flags, errstr);

	const unsigned int bogus_cmd = 0xdeadc0de;
	k_process_madvise(-1, iov + 1, 2, bogus_cmd, 0);
	printf("process_madvise(-1, [{iov_base=%p, iov_len=%lu}, ... /* %p */]"
	       ", 2, %#x /* MADV_??? */, 0) = %s\n",
	       iov[1].iov_base, (unsigned long) iov[1].iov_len,
	       iov + 2, bogus_cmd, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
