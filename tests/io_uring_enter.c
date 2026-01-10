/*
 * Check decoding of io_uring_enter syscall.
 *
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "kernel_timespec.h"
#include "scno.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kernel_time_types.h"
#define UAPI_LINUX_IO_URING_H_SKIP_LINUX_TIME_TYPES_H
#include <linux/io_uring.h>

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
	TAIL_ALLOC_OBJECT_CONST_PTR(struct io_uring_getevents_arg, arg);
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_timespec64_t, ts);
	sigset_t mask;

	memset(&mask, -1, sizeof(mask));
	sigdelset(&mask, SIGHUP);
	sigdelset(&mask, SIGKILL);
	sigdelset(&mask, SIGSTOP);
	memcpy(sigmask, &mask, size);

	const unsigned int to_submit = 0xdeadbeef;
	const unsigned int min_complete = 0xcafef00d;
	const unsigned long offset = (unsigned long) 0xdefacedfacefeedULL;

	sys_io_uring_enter(fd, to_submit, min_complete, -1U, sigmask, size);
	printf("io_uring_enter(%u<%s>, %u, %u"
	       ", IORING_ENTER_GETEVENTS|IORING_ENTER_SQ_WAKEUP"
	       "|IORING_ENTER_SQ_WAIT|IORING_ENTER_EXT_ARG"
	       "|IORING_ENTER_REGISTERED_RING"
	       "|IORING_ENTER_ABS_TIMER"
	       "|IORING_ENTER_EXT_ARG_REG"
	       "|IORING_ENTER_NO_IOWAIT"
	       "|%#x, %s, %u) = %s\n",
	       fd, path, to_submit, min_complete, -1U - 255U,
	       "~[HUP KILL STOP]", size, errstr);

	/* Test IORING_ENTER_EXT_ARG */
	memset(arg, 0, sizeof(*arg));
	arg->sigmask = (uintptr_t) sigmask;
	arg->sigmask_sz = size;
	arg->min_wait_usec = 0xdeadc0de;
	arg->ts = (uintptr_t) ts;
	ts->tv_sec = (typeof(ts->tv_sec)) 0xfacefed1facefed2ULL;
        ts->tv_nsec = (typeof(ts->tv_nsec)) 0xfacefed3facefed4ULL;

	sys_io_uring_enter(fd, to_submit, min_complete,
			   IORING_ENTER_GETEVENTS | IORING_ENTER_EXT_ARG,
			   arg, sizeof(*arg));
	const char *expected_sigmask = "~[HUP KILL STOP]";
	printf("io_uring_enter(%u<%s>, %u, %u"
	       ", IORING_ENTER_GETEVENTS|IORING_ENTER_EXT_ARG"
	       ", {sigmask=%s, sigmask_sz=%u, min_wait_usec=%u"
	       ", ts={tv_sec=%lld, tv_nsec=%llu}}"
	       ", %zu) = %s\n",
	       fd, path, to_submit, min_complete,
	       expected_sigmask, arg->sigmask_sz, arg->min_wait_usec,
	       (long long) ts->tv_sec, zero_extend_signed_to_ull(ts->tv_nsec),
	       sizeof(*arg), errstr);

	/* Test IORING_ENTER_EXT_ARG_REG */
	sys_io_uring_enter(fd, to_submit, min_complete,
			   IORING_ENTER_GETEVENTS | IORING_ENTER_EXT_ARG_REG,
			   (void *) offset, sizeof(struct io_uring_reg_wait));
	printf("io_uring_enter(%u<%s>, %u, %u"
	       ", IORING_ENTER_GETEVENTS|IORING_ENTER_EXT_ARG_REG"
	       ", %lu, %zu) = %s\n",
	       fd, path, to_submit, min_complete,
	       offset, sizeof(struct io_uring_reg_wait), errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
