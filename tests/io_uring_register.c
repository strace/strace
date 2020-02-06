/*
 * Check decoding of io_uring_register syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <unistd.h>
#include "scno.h"

#ifdef __NR_io_uring_register

# include <fcntl.h>
# include <inttypes.h>
# include <stdio.h>
# include <string.h>
# include <sys/uio.h>

# ifdef HAVE_LINUX_IO_URING_H
#  include <linux/io_uring.h>
# endif

/* From tests/bpf.c */
#if defined MPERS_IS_m32 || SIZEOF_KERNEL_LONG_T > 4
# define BIG_ADDR_MAYBE(addr_)
#elif defined __arm__ || defined __i386__ || defined __mips__ \
   || defined __powerpc__ || defined __riscv__ || defined __s390__ \
   || defined __sparc__ || defined __tile__
# define BIG_ADDR_MAYBE(addr_) addr_ " or "
#else
# define BIG_ADDR_MAYBE(addr_)
#endif


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

	static const unsigned int invalid_ops[] = { 0xbadc0dedU, 7 };

	for (size_t i = 0; i < ARRAY_SIZE(invalid_ops); i++) {
		sys_io_uring_register(fd_null, invalid_ops[i], path_null,
				      0xdeadbeef);
		printf("io_uring_register(%u<%s>, %#x /* IORING_REGISTER_??? */"
		       ", %p, %u) = %s\n",
		       fd_null, path_null, invalid_ops[i], path_null,
		       0xdeadbeef, errstr);
	}

	static const struct {
		unsigned int op;
		const char *str;
	} no_arg_ops[] = {
		{ 1, "IORING_UNREGISTER_BUFFERS" },
		{ 3, "IORING_UNREGISTER_FILES" },
		{ 5, "IORING_UNREGISTER_EVENTFD" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(no_arg_ops); i++) {
		sys_io_uring_register(fd_null, no_arg_ops[i].op, path_null,
				      0xdeadbeef);
		printf("io_uring_register(%u<%s>, %s, %p, %u) = %s\n",
		       fd_null, path_null, no_arg_ops[i].str, path_null,
		       0xdeadbeef, errstr);
	}

	sys_io_uring_register(fd_null, 0, arg_iov, ARRAY_SIZE(iov));
	printf("io_uring_register(%u<%s>, IORING_REGISTER_BUFFERS"
	       ", [{iov_base=%p, iov_len=%lu}, {iov_base=%p, iov_len=%lu}]"
	       ", %u) = %s\n",
	       fd_null, path_null, iov[0].iov_base,
	       (unsigned long) iov[0].iov_len,
	       iov[1].iov_base, (unsigned long) iov[1].iov_len,
	       (unsigned int) ARRAY_SIZE(iov), errstr);

	static const struct {
		unsigned int op;
		const char *str;
	} fd_arr_ops[] = {
		{ 2, "IORING_REGISTER_FILES" },
		{ 4, "IORING_REGISTER_EVENTFD" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(fd_arr_ops); i++) {
		sys_io_uring_register(fd_null, fd_arr_ops[i].op, arg_fds,
				      ARRAY_SIZE(fds));
		printf("io_uring_register(%u<%s>, %s, [%u<%s>, %u<%s>], %u)"
		       " = %s\n",
		       fd_null, path_null, fd_arr_ops[i].str,
		       fd_full, path_full, fd_null, path_null,
		       (unsigned int) ARRAY_SIZE(fds), errstr);
	}

#ifdef HAVE_STRUCT_IO_URING_FILES_UPDATE
	struct io_uring_files_update bogus_iufu;
	struct io_uring_files_update iufu;

	sys_io_uring_register(fd_null, 6, NULL, 0xfacefeed);
	printf("io_uring_register(%u<%s>, IORING_REGISTER_FILES_UPDATE"
	       ", NULL, 4207869677) = %s\n",
	       fd_null, path_null, errstr);

	fill_memory(&bogus_iufu, sizeof(bogus_iufu));
	sys_io_uring_register(fd_null, 6, &bogus_iufu, 0);
	printf("io_uring_register(%u<%s>, IORING_REGISTER_FILES_UPDATE"
	       ", {offset=%" PRIu32 ", resv=%#" PRIx32 ", fds="
	       BIG_ADDR_MAYBE("0x8f8e8d8c8b8a8988") "[]}, 0) = %s\n",
	       fd_null, path_null,
	       ((uint32_t *) &bogus_iufu)[0], ((uint32_t *) &bogus_iufu)[1],
	       errstr);

	memset(&iufu, 0, sizeof(iufu));
	iufu.offset = 0xdeadc0deU;
	iufu.fds = (uintptr_t) fds;
	sys_io_uring_register(fd_null, 6, &iufu, ARRAY_SIZE(fds));
	printf("io_uring_register(%u<%s>, IORING_REGISTER_FILES_UPDATE"
	       ", {offset=3735929054, fds=[%u<%s>, %u<%s>]}, %u) = %s\n",
	       fd_null, path_null, fd_full, path_full, fd_null, path_null,
	       (unsigned int) ARRAY_SIZE(fds), errstr);
#endif

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_io_uring_register")

#endif
