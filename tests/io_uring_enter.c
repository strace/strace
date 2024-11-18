/*
 * Check decoding of io_uring_enter syscall.
 *
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char *errstr;

static long
sys_io_uring_enter(unsigned int fd, unsigned int to_submit,
		   unsigned int min_complete, unsigned int flags,
		   const void *sigset_addr, kernel_ulong_t sigset_size)

{
	kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	kernel_ulong_t arg1 = fill | fd;
	kernel_ulong_t arg2 = fill | to_submit;
	kernel_ulong_t arg3 = fill | min_complete;
	kernel_ulong_t arg4 = fill | flags;
	kernel_ulong_t arg5 = (unsigned long) sigset_addr;
	kernel_ulong_t arg6 = sigset_size;

	long rc = syscall(__NR_io_uring_enter,
			  arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const char path[] = "/dev/null";

	skip_if_unavailable("/proc/self/fd/");

	int fd = open(path, O_RDONLY);
	if (fd < 0)
		perror_msg_and_fail("open: %s", path);

	const unsigned int size = get_sigset_size();
	void *const sigmask = tail_alloc(size);
	sigset_t mask;

	memset(&mask, -1, sizeof(mask));
	sigdelset(&mask, SIGHUP);
	sigdelset(&mask, SIGKILL);
	sigdelset(&mask, SIGSTOP);
	memcpy(sigmask, &mask, size);

	const unsigned int to_submit = 0xdeadbeef;
	const unsigned int min_complete = 0xcafef00d;

	sys_io_uring_enter(fd, to_submit, min_complete, -1U, sigmask, size);
	printf("io_uring_enter(%u<%s>, %u, %u"
	       ", IORING_ENTER_GETEVENTS|IORING_ENTER_SQ_WAKEUP"
	       "|IORING_ENTER_SQ_WAIT|IORING_ENTER_EXT_ARG"
	       "|IORING_ENTER_REGISTERED_RING"
	       "|IORING_ENTER_ABS_TIMER"
	       "|%#x, %s, %u) = %s\n",
	       fd, path, to_submit, min_complete, -1U - 63U,
	       "~[HUP KILL STOP]", size, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
