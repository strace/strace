/*
 * Check decoding of io_uring_register syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <unistd.h>
#include "scno.h"

#ifdef __NR_io_uring_register

# include <fcntl.h>
# include <stdio.h>
# include <string.h>
# include <sys/uio.h>

static const char *errstr;

static long
sys_io_uring_register(unsigned int fd, unsigned int opcode,
		      const void *arg, unsigned int nargs)

{
	kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	kernel_ulong_t arg1 = fill | fd;
	kernel_ulong_t arg2 = fill | opcode;
	kernel_ulong_t arg3 = (unsigned long) arg;
	kernel_ulong_t arg4 = fill | nargs;

	long rc = syscall(__NR_io_uring_register,
			  arg1, arg2, arg3, arg4, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const char path_null[] = "/dev/null";
	static const char path_full[] = "/dev/full";
	const struct iovec iov[] = {
		{
			.iov_base = (void *) (unsigned long) 0xfacefeedcafef00d,
			.iov_len = (unsigned long) 0xdeadfacebeefcafe
		},
		{
			.iov_base = (void *) path_null,
			.iov_len = sizeof(path_null)
		}
	};
	const struct iovec *arg_iov = tail_memdup(iov, sizeof(iov));

	skip_if_unavailable("/proc/self/fd/");

	int fd_null = open(path_null, O_RDONLY);
	if (fd_null < 0)
		perror_msg_and_fail("open: %s", path_null);

	int fd_full = open(path_full, O_RDONLY);
	if (fd_full < 0)
		perror_msg_and_fail("open: %s", path_null);

	int fds[] = { fd_full, fd_null };
	const int *arg_fds = tail_memdup(fds, sizeof(fds));

	sys_io_uring_register(fd_null, 0xbadc0ded, path_null, 0xdeadbeef);
	printf("io_uring_register(%u<%s>, %#x /* IORING_REGISTER_??? */"
	       ", %p, %u) = %s\n",
	       fd_null, path_null, 0xbadc0ded, path_null, 0xdeadbeef, errstr);

	sys_io_uring_register(fd_null, 0, arg_iov, ARRAY_SIZE(iov));
	printf("io_uring_register(%u<%s>, IORING_REGISTER_BUFFERS"
	       ", [{iov_base=%p, iov_len=%lu}, {iov_base=%p, iov_len=%lu}]"
	       ", %u) = %s\n",
	       fd_null, path_null, iov[0].iov_base,
	       (unsigned long) iov[0].iov_len,
	       iov[1].iov_base, (unsigned long) iov[1].iov_len,
	       (unsigned int) ARRAY_SIZE(iov), errstr);

	sys_io_uring_register(fd_null, 2, arg_fds, ARRAY_SIZE(fds));
	printf("io_uring_register(%u<%s>, IORING_REGISTER_FILES"
	       ", [%u<%s>, %u<%s>], %u) = %s\n",
	       fd_null, path_null, fd_full, path_full, fd_null, path_null,
	       (unsigned int) ARRAY_SIZE(fds), errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_io_uring_register")

#endif
