/*
 * Check decoding of io_uring_register syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kernel_time_types.h"
#define UAPI_LINUX_IO_URING_H_SKIP_LINUX_TIME_TYPES_H
#include <linux/io_uring.h>

/* From tests/bpf.c */
#if defined MPERS_IS_m32 || SIZEOF_KERNEL_LONG_T > 4
# define BIG_ADDR_MAYBE(addr_)
# define BIG_ADDR_MASK 0
#elif defined __arm__ || defined __i386__ || defined __mips__ \
   || defined __powerpc__ || defined __riscv || defined __s390__ \
   || defined __sparc__ || defined __tile__
# define BIG_ADDR_MAYBE(addr_) addr_ " or "
# define BIG_ADDR_MASK 0xffffffff00000000ULL
#else
# define BIG_ADDR_MAYBE(addr_)
# define BIG_ADDR_MASK 0
#endif

#ifndef RETVAL_INJECTED
# define RETVAL_INJECTED 0
#endif

#if RETVAL_INJECTED
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif

#define ARR_ITEM(arr_, idx_) ((arr_)[(idx_) % ARRAY_SIZE(arr_)])

static const char path_null[] = "/dev/null";
static const char path_full[] = "/dev/full";

char errstr[1024];

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
	snprintf(errstr, sizeof(errstr), "%s%s", sprintrc(rc), INJ_STR);
	return rc;
}

static void
print_rsrc_data(const struct iovec *arg_iov, const struct iovec *iov,
		const int *arg_fds, const int *fds, const size_t i,
		const size_t j, const void *endptr, bool upd)
{
	printf(", data=");
	if (BIG_ADDR_MASK && (j & 2)) {
		printf("%#llx or ", (unsigned long long) (BIG_ADDR_MASK |
		       (j & 1 ? i ? (uintptr_t) arg_iov
				  : (uintptr_t) arg_fds : 0)));
	}
	if (!(j & 1)) {
		printf("NULL");
		return;
	}

	printf("[");
	if (!(j & 32))
		goto print_rsrc_data_end;
	if (i) {
		printf("{iov_base=%p, iov_len=%lu}, {iov_base=%p, iov_len=%lu}",
		       iov[0].iov_base, (unsigned long) iov[0].iov_len,
		       iov[1].iov_base, (unsigned long) iov[1].iov_len);
	} else {
		printf("%u<%s>, %u<%s>, -1, %s, -3",
		       fds[0], path_full, fds[1], path_null,
		       upd ? XLAT_KNOWN(-2, "IORING_REGISTER_FILES_SKIP")
			   : "-2");
	}
	if (j & 16)
		printf(", ... /* %p */", endptr);
print_rsrc_data_end:
	printf("]");
}

static void
print_rsrc_tags(const uint64_t *arg_tags, const uint64_t *tags, const size_t i,
		const size_t j, const void *endptr)
{
	printf(", tags=");
	if (BIG_ADDR_MASK && (j & 8)) {
		printf("%#llx or ", (unsigned long long) (
		       BIG_ADDR_MASK | (j & 4 ? 0
			: (uintptr_t) (arg_tags + 1  + i * 3 - !!(j & 64)))));
	}
	if (j & 4) {
		printf("NULL");
		return;
	}

	printf("[");
	if (!(j & 32))
		goto print_rsrc_tags_end;
	if (i) {
		printf("%s0xbadc0deddadfaced%s",
		       j & 64 ? "0xfacefeed, " : "", j & 64 ? "" : ", 0");
	} else {
		printf("%s0x1, 0xdead, 0xfacefeed, 0xbadc0deddadfaced%s",
		       j & 64 ? "0x1337, " : "", j & 64 ? "" : ", 0");
	}
	if (j & 16) {
		if (j & 64)
			printf(", 0");
		else
			printf(", ... /* %p */", endptr);
	}
print_rsrc_tags_end:
	printf("]");
}

int
main(void)
{
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

	close(0); /* Trying to get fd 0 for /dev/null */
	int fd_null = open(path_null, O_RDONLY);
	if (fd_null < 0)
		perror_msg_and_fail("open: %s", path_null);

	int fd_full = open(path_full, O_RDONLY);
	if (fd_full < 0)
		perror_msg_and_fail("open: %s", path_full);

	int fds[] = { fd_full, fd_null, -1, -2, -3 };
	const char *paths[ARRAY_SIZE(fds)] = { path_full, path_null };
	const int *arg_fds = tail_memdup(fds, sizeof(fds));


	/* Invalid op */
	static const unsigned int invalid_ops[] = { 0xbadc0dedU, 24 };

	for (size_t i = 0; i < ARRAY_SIZE(invalid_ops); i++) {
		sys_io_uring_register(fd_null, invalid_ops[i], path_null,
				      0xdeadbeef);
		printf("io_uring_register(%u<%s>, %#x"
		       NRAW(" /* IORING_REGISTER_??? */") ", %p, %u) = %s\n",
		       fd_null, path_null, invalid_ops[i], path_null,
		       0xdeadbeef, errstr);
	}


	/* Operations without an argument */
	static const struct {
		unsigned int op;
		const char *str;
	} no_arg_ops[] = {
		{ 1, "IORING_UNREGISTER_BUFFERS" },
		{ 3, "IORING_UNREGISTER_FILES" },
		{ 5, "IORING_UNREGISTER_EVENTFD" },
		{ 9, "IORING_REGISTER_PERSONALITY" },
		{ 10, "IORING_UNREGISTER_PERSONALITY" },
		{ 12, "IORING_REGISTER_ENABLE_RINGS" },
		{ 18, "IORING_UNREGISTER_IOWQ_AFF" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(no_arg_ops); i++) {
		sys_io_uring_register(fd_null, no_arg_ops[i].op, path_null,
				      0xdeadbeef);
		printf("io_uring_register(%u<%s>, " XLAT_FMT ", %p, %u) = %s\n",
		       fd_null, path_null,
		       XLAT_SEL(no_arg_ops[i].op, no_arg_ops[i].str),
		       path_null, 0xdeadbeef, errstr);
	}


	/* IORING_REGISTER_BUFFERS */
	sys_io_uring_register(fd_null, 0, arg_iov, ARRAY_SIZE(iov));
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0, "IORING_REGISTER_BUFFERS")
	       ", [{iov_base=%p, iov_len=%lu}, {iov_base=%p, iov_len=%lu}]"
	       ", %u) = %s\n",
	       fd_null, path_null, iov[0].iov_base,
	       (unsigned long) iov[0].iov_len,
	       iov[1].iov_base, (unsigned long) iov[1].iov_len,
	       (unsigned int) ARRAY_SIZE(iov), errstr);


	/* Operations with an fd array argument */
	static const struct {
		unsigned int op;
		const char *str;
	} fd_arr_ops[] = {
		{ 2, "IORING_REGISTER_FILES" },
		{ 4, "IORING_REGISTER_EVENTFD" },
		{ 7, "IORING_REGISTER_EVENTFD_ASYNC" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(fd_arr_ops); i++) {
		sys_io_uring_register(fd_null, fd_arr_ops[i].op, arg_fds,
				      ARRAY_SIZE(fds));
		printf("io_uring_register(%u<%s>, " XLAT_FMT
		       ", [%u<%s>, %u<%s>, -1, -2, -3], %u) = %s\n",
		       fd_null, path_null,
		       XLAT_SEL(fd_arr_ops[i].op, fd_arr_ops[i].str),
		       fd_full, path_full, fd_null, path_null,
		       (unsigned int) ARRAY_SIZE(fds), errstr);
	}


	/* IORING_REGISTER_FILES_UPDATE */
	struct io_uring_files_update bogus_iufu;
	struct io_uring_files_update iufu;

	sys_io_uring_register(fd_null, 6, NULL, 0xfacefeed);
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x6, "IORING_REGISTER_FILES_UPDATE")
	       ", NULL, 4207869677) = %s\n",
	       fd_null, path_null, errstr);

	fill_memory(&bogus_iufu, sizeof(bogus_iufu));
	sys_io_uring_register(fd_null, 6, &bogus_iufu, 0);
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x6, "IORING_REGISTER_FILES_UPDATE")
	       ", {offset=%" PRIu32 ", resv=%#" PRIx32 ", fds="
	       BIG_ADDR_MAYBE(BE_LE("0x88898a8b8c8d8e8f", "0x8f8e8d8c8b8a8988"))
	       "[]}, 0) = %s\n",
	       fd_null, path_null,
	       ((uint32_t *) &bogus_iufu)[0], ((uint32_t *) &bogus_iufu)[1],
	       errstr);

	memset(&iufu, 0, sizeof(iufu));
	iufu.offset = 0xdeadc0deU;
	iufu.fds = (uintptr_t) fds;
	sys_io_uring_register(fd_null, 6, &iufu, ARRAY_SIZE(fds));
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x6, "IORING_REGISTER_FILES_UPDATE")
	       ", {offset=3735929054, fds=[%u<%s>, %u<%s>, -1, "
	       XLAT_KNOWN(-2, "IORING_REGISTER_FILES_SKIP")
	       ", -3]}, %u) = %s\n",
	       fd_null, path_null, fd_full, path_full, fd_null, path_null,
	       (unsigned int) ARRAY_SIZE(fds), errstr);

	struct io_uring_probe *probe = tail_alloc(sizeof(*probe) +
		       (DEFAULT_STRLEN + 1) * sizeof(struct io_uring_probe_op));


	/* IORING_REGISTER_PROBE */
	sys_io_uring_register(fd_null, IORING_REGISTER_PROBE, NULL, 0xfacefeed);
	printf("io_uring_register(%u<%s>, " XLAT_FMT
	       ", NULL, 4207869677) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE), errstr);

	sys_io_uring_register(fd_null, IORING_REGISTER_PROBE, probe,
			      0xfacefeed);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", %p, 4207869677) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE), probe,
	       errstr);

	sys_io_uring_register(fd_null, IORING_REGISTER_PROBE,
			      (char *) probe + 1, DEFAULT_STRLEN + 1);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", %p, %d) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE),
	       (char *) probe + 1, DEFAULT_STRLEN + 1, errstr);

	sys_io_uring_register(fd_null, IORING_REGISTER_PROBE, probe, 0);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", {last_op=%u"
	       NRAW(" /* IORING_OP_??? */") ", ops_len=%hhu, resv=%#hx"
	       ", resv2=[%#x, %#x, %#x], ops=[]}"
#if RETVAL_INJECTED
	       " => {last_op=%u" NRAW(" /* IORING_OP_??? */") ", ops_len=%hhu"
	       ", resv=%#hx, resv2=[%#x, %#x, %#x], ops=[...]}"
#endif
	       ", 0) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE),
	       probe->last_op, probe->ops_len, probe->resv,
	       probe->resv2[0], probe->resv2[1], probe->resv2[2],
#if RETVAL_INJECTED
	       probe->last_op, probe->ops_len, probe->resv,
	       probe->resv2[0], probe->resv2[1], probe->resv2[2],
#endif
	       errstr);

	probe->last_op = IORING_OP_READV;
	probe->resv = 0;
	sys_io_uring_register(fd_null, IORING_REGISTER_PROBE, probe, 0);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", {last_op=" XLAT_FMT_U
	       ", ops_len=%hhu, resv2=[%#x, %#x, %#x], ops=[]}"
#if RETVAL_INJECTED
	       " => {last_op=" XLAT_FMT_U ", ops_len=%hhu"
	       ", resv2=[%#x, %#x, %#x], ops=[...]}"
#endif
	       ", 0) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE),
	       XLAT_ARGS(IORING_OP_READV), probe->ops_len,
	       probe->resv2[0], probe->resv2[1], probe->resv2[2],
#if RETVAL_INJECTED
	       XLAT_ARGS(IORING_OP_READV), probe->ops_len,
	       probe->resv2[0], probe->resv2[1], probe->resv2[2],
#endif
	       errstr);

	probe->last_op = IORING_OP_EPOLL_CTL;
	probe->resv2[0] = 0;
	probe->resv2[2] = 0;

	probe->ops[0].op = IORING_OP_NOP;
	probe->ops[0].resv = 0xde;
	probe->ops[0].flags = 0;
	probe->ops[0].resv2 = 0xbeefface;

	probe->ops[1].op = 53;
	probe->ops[1].resv = 0;
	probe->ops[1].flags = IO_URING_OP_SUPPORTED;
	probe->ops[1].resv2 = 0xdeadc0de;

	probe->ops[2].op = 54;
	probe->ops[2].resv = 0xaf;
	probe->ops[2].flags = 0xbeef;
	probe->ops[2].resv2 = 0;

	probe->ops[3].op = 0xfe;
	probe->ops[3].resv = 0;
	probe->ops[3].flags = 0xc0de;
	probe->ops[3].resv2 = 0;

	sys_io_uring_register(fd_null, IORING_REGISTER_PROBE, probe, 4);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", {last_op=" XLAT_FMT_U
	       ", ops_len=%hhu, resv2=[0, %#x, 0], ops=["
	       "{op=" XLAT_FMT_U ", resv=0xde, flags=0, resv2=0xbeefface}, "
	       "{op=" XLAT_FMT_U ", flags=" XLAT_FMT ", resv2=0xdeadc0de}, "
	       "{op=54" NRAW(" /* IORING_OP_??? */") ", resv=0xaf, flags="
	       XLAT_FMT "}, {op=254" NRAW(" /* IORING_OP_??? */")
	       ", flags=0xc0de" NRAW(" /* IO_URING_OP_??? */") "}]}"
#if RETVAL_INJECTED
	       " => {last_op=" XLAT_FMT_U ", ops_len=%hhu, resv2=[0, %#x, 0], "
	       "ops=[{op=" XLAT_FMT_U ", resv=0xde, flags=0, resv2=0xbeefface}"
	       ", {op=" XLAT_FMT_U ", flags=" XLAT_FMT ", resv2=0xdeadc0de}"
	       ", {op=54" NRAW(" /* IORING_OP_??? */") ", resv=0xaf, flags="
	       XLAT_FMT "}, {op=254" NRAW(" /* IORING_OP_??? */")
	       ", flags=0xc0de" NRAW(" /* IO_URING_OP_??? */") "}, ...]}"
#endif
	       ", 4) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE),
	       XLAT_ARGS(IORING_OP_EPOLL_CTL), probe->ops_len, probe->resv2[1],
	       XLAT_ARGS(IORING_OP_NOP), XLAT_ARGS(IORING_OP_FUTEX_WAITV),
	       XLAT_ARGS(IO_URING_OP_SUPPORTED),
	       XLAT_ARGS(IO_URING_OP_SUPPORTED|0xbeee),
#if RETVAL_INJECTED
	       XLAT_ARGS(IORING_OP_EPOLL_CTL), probe->ops_len, probe->resv2[1],
	       XLAT_ARGS(IORING_OP_NOP), XLAT_ARGS(IORING_OP_FUTEX_WAITV),
	       XLAT_ARGS(IO_URING_OP_SUPPORTED),
	       XLAT_ARGS(IO_URING_OP_SUPPORTED|0xbeee),
#endif
	       errstr);

	probe->last_op = 54;
	probe->resv2[1] = 0;
	fill_memory_ex(probe->ops, sizeof(probe->ops[0]) * (DEFAULT_STRLEN + 1),
		    0x40, 0x80);
	sys_io_uring_register(fd_null, IORING_REGISTER_PROBE, probe,
			      DEFAULT_STRLEN + 1);
	printf("io_uring_register(%u<%s>, " XLAT_FMT,
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE));
	for (size_t c = 0; c < 1 + RETVAL_INJECTED; c++) {
		printf("%s{last_op=54" NRAW(" /* IORING_OP_??? */")
		       ", ops_len=%hhu, ops=[",
		       c ? " => " : ", ", probe->ops_len);
		for (size_t i = 0; i < DEFAULT_STRLEN; i++) {
			printf("%s{op=%u" NRAW(" /* IORING_OP_??? */")
			       ", resv=%#hhx, flags=",
			       i ? ", " : "", probe->ops[i].op,
			       probe->ops[i].resv);
#if XLAT_RAW
			printf("%#hx",
			       (typeof(probe->ops[i].flags))
					(probe->ops[i].flags));
#else /* !XLAT_RAW */
			if (probe->ops[i].flags & 1) {
				printf(VERB("%#hx /* ") "IO_URING_OP_SUPPORTED"
				       "|%#hx" VERB(" */"),
# if XLAT_VERBOSE
				       probe->ops[i].flags,
# endif
				       (uint16_t) (probe->ops[i].flags & ~1));
			} else {
				printf("%#hx /* IO_URING_OP_??? */",
				       probe->ops[i].flags);
			}
#endif /* XLAT_RAW */
			printf(", resv2=%#x}", probe->ops[i].resv2);
		}
		printf(", ...]}");
	}
	printf(", %d) = %s\n", DEFAULT_STRLEN + 1, errstr);

	probe->last_op = 0;
	probe->ops_len = 0;
	memset(probe->ops, 0, sizeof(probe->ops[0]) * (DEFAULT_STRLEN + 1));
	sys_io_uring_register(fd_null, IORING_REGISTER_PROBE, probe, 8);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", "
#if RETVAL_INJECTED
	       "{last_op=" XLAT_KNOWN(0, "IORING_OP_NOP") ", ops_len=0, ops=[]}"
#else
	       "%p"
#endif
	       ", 8) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE),
#if !RETVAL_INJECTED
	       probe,
#endif
	       errstr);


	/* IORING_REGISTER_RESTRICTIONS */
	static const struct {
		uint16_t    opcode;
		const char *opcode_str;
		bool        opcode_known;
		const char *op_pfx;
		uint8_t     op;
		const char *op_str;
		bool op_known;
	} restrictions_data[] = {
		{ ARG_STR(IORING_RESTRICTION_REGISTER_OP), true,
		  "register_op=", ARG_STR(IORING_REGISTER_BUFFERS), true },
		{ ARG_STR(IORING_RESTRICTION_REGISTER_OP), true,
		  "register_op=", ARG_STR(IORING_UNREGISTER_PBUF_RING),
		  true },
		{ ARG_STR(IORING_RESTRICTION_REGISTER_OP), true,
		  "register_op=", 24, " /* IORING_REGISTER_??? */", false },
		{ ARG_STR(IORING_RESTRICTION_REGISTER_OP), true,
		  "register_op=", 255, " /* IORING_REGISTER_??? */", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_OP), true,
		  "sqe_op=", ARG_STR(IORING_OP_NOP), true },
		{ ARG_STR(IORING_RESTRICTION_SQE_OP), true,
		  "sqe_op=", ARG_STR(IORING_OP_FUTEX_WAITV), true },
		{ ARG_STR(IORING_RESTRICTION_SQE_OP), true,
		  "sqe_op=", 54, " /* IORING_OP_??? */", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_OP), true,
		  "sqe_op=", 255, " /* IORING_OP_??? */", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_ALLOWED), true,
		  "sqe_flags=", 0, "", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_ALLOWED), true,
		  "sqe_flags=", 64, "IOSQE_CQE_SKIP_SUCCESS", true },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_ALLOWED), true,
		  "sqe_flags=", 0xff, "IOSQE_FIXED_FILE|IOSQE_IO_DRAIN"
				      "|IOSQE_IO_LINK|IOSQE_IO_HARDLINK"
				      "|IOSQE_ASYNC|IOSQE_BUFFER_SELECT"
				      "|IOSQE_CQE_SKIP_SUCCESS"
				      "|0x80",
		  true },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_ALLOWED), true,
		  "sqe_flags=", 128, " /* IOSQE_??? */", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_REQUIRED), true,
		  "sqe_flags=", 0, "", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_REQUIRED), true,
		  "sqe_flags=", 1, "IOSQE_FIXED_FILE", true },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_REQUIRED), true,
		  "sqe_flags=", 192, "IOSQE_CQE_SKIP_SUCCESS|0x80", true },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_REQUIRED), true,
		  "sqe_flags=",	128, " /* IOSQE_??? */", false },
		{ 4, " /* IORING_RESTRICTION_??? */", false, "", 0 },
		{ 4, " /* IORING_RESTRICTION_??? */", false, "", 239 },
		{ 137, " /* IORING_RESTRICTION_??? */", false, "", 0 },
	};
	struct io_uring_restriction *restrictions =
			tail_alloc(sizeof(*restrictions)
				   * ARRAY_SIZE(restrictions_data));
	char *restrictions_end = (char *) (restrictions
					   + ARRAY_SIZE(restrictions_data));

	sys_io_uring_register(fd_null, IORING_REGISTER_RESTRICTIONS, NULL,
			      0xfacefeed);
	printf("io_uring_register(%u<%s>, " XLAT_FMT
	       ", NULL, 4207869677) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_RESTRICTIONS),
	       errstr);

	sys_io_uring_register(fd_null, IORING_REGISTER_RESTRICTIONS,
			      restrictions_end, 0);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", [], 0) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_RESTRICTIONS),
	       errstr);

	sys_io_uring_register(fd_null, IORING_REGISTER_RESTRICTIONS,
			      restrictions_end, 1);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", %p, 1) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_RESTRICTIONS),
	       restrictions_end, errstr);

	sys_io_uring_register(fd_null, IORING_REGISTER_RESTRICTIONS,
			      restrictions_end - sizeof(*restrictions) + 1, 1);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", %p, 1) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_RESTRICTIONS),
	       restrictions_end - sizeof(*restrictions) + 1, errstr);

	struct io_uring_restriction *p =
			restrictions + ARRAY_SIZE(restrictions_data) - 1;
	for (size_t i = 0; i < ARRAY_SIZE(restrictions_data); i++) {
		memset(p, 0, sizeof(*restrictions));
		p->opcode = restrictions_data[i].opcode;
		p->sqe_flags = restrictions_data[i].op;

		sys_io_uring_register(fd_null, IORING_REGISTER_RESTRICTIONS,
				      p, 1);
		printf("io_uring_register(%u<%s>, " XLAT_FMT ", [{opcode=",
		       fd_null, path_null,
		       XLAT_ARGS(IORING_REGISTER_RESTRICTIONS));
		if (restrictions_data[i].opcode_known) {
			printf(XLAT_FMT,
			       XLAT_SEL(restrictions_data[i].opcode,
					restrictions_data[i].opcode_str));
			printf(", %s", restrictions_data[i].op_pfx);
			if (restrictions_data[i].op_known) {
				printf(XLAT_FMT,
				       XLAT_SEL(restrictions_data[i].op,
						restrictions_data[i].op_str));
			} else {
				printf("%#x%s",
				       restrictions_data[i].op,
				       XLAT_RAW ? ""
						: restrictions_data[i].op_str);
			}
		} else {
			printf("%#x%s /* op: %#x */",
			       restrictions_data[i].opcode,
			       XLAT_RAW ? "" : restrictions_data[i].opcode_str,
			       restrictions_data[i].op);
		}
		printf("}], 1) = %s\n", errstr);
	}

	for (size_t i = 0; i < ARRAY_SIZE(restrictions_data); i++) {
		restrictions[i].opcode = restrictions_data[i].opcode;
		restrictions[i].sqe_op = restrictions_data[i].op;
		restrictions[i].resv = i & 1 ? 0x70 + i * 7 : 0;
		restrictions[i].resv2[0] = i & 2 ? 0x80808080 | i * 0x1020304
						 : 0;
		restrictions[i].resv2[1] = i & 4 ? 0x80808080 | i * 0x4030201
						 : 0;
		restrictions[i].resv2[2] = i & 8 ? 0x08080808 | i * 0x40302010
						 : 0;
	}

	for (size_t j = 0; j < 3; j++) {
		if (j == 2) {
			memmove(((char *) restrictions) + 4, restrictions,
				sizeof(*restrictions)
				* ARRAY_SIZE(restrictions_data) - 4);
		}

		sys_io_uring_register(fd_null, IORING_REGISTER_RESTRICTIONS,
				      ((char *) restrictions) + 4 * !!(j == 2),
				      ARRAY_SIZE(restrictions_data)
				      + !!(j == 1));
		printf("io_uring_register(%u<%s>, " XLAT_FMT ", [",
		       fd_null, path_null,
		       XLAT_ARGS(IORING_REGISTER_RESTRICTIONS));

		for (unsigned int i = 0;
		     i < ARRAY_SIZE(restrictions_data) - !!(j == 2); i++) {
			printf("%s{opcode=", i ? ", " : "");
			if (restrictions_data[i].opcode_known) {
				printf(XLAT_FMT,
				       XLAT_SEL(restrictions_data[i].opcode,
						restrictions_data[i].opcode_str));
				printf(", %s", restrictions_data[i].op_pfx);
				if (restrictions_data[i].op_known) {
					printf(XLAT_FMT,
					       XLAT_SEL(restrictions_data[i].op,
							restrictions_data[i].op_str));
				} else {
					printf("%#x%s",
					       restrictions_data[i].op,
					       XLAT_RAW ? ""
						 : restrictions_data[i].op_str);
				}
			} else {
				printf("%#x%s /* op: %#x */",
				       restrictions_data[i].opcode,
				       XLAT_RAW ? ""
					     : restrictions_data[i].opcode_str,
				       restrictions_data[i].op);
			}

			if (i & 1) {
				printf(", resv=%#hhx",
				       (unsigned char) (0x70 + i * 7));
			}
			if (i & 0xe) {
				printf(", resv2=[%#x, %#x, %#x]",
				       i & 2 ? 0x80808080 | i * 0x1020304 : 0,
				       i & 4 ? 0x80808080 | i * 0x4030201 : 0,
				       i & 8 ? 0x08080808 | i * 0x40302010 : 0);
			}

			printf("}");
		}

		if (j) {
			printf(", ... /* %p */",
			       j == 1 ? restrictions_end
				      : restrictions_end - sizeof(*restrictions)
				        + 4);
		}
		printf("], %zu) = %s\n",
		       ARRAY_SIZE(restrictions_data) + !!(j == 1), errstr);
	}


	/* IORING_REGISTER_FILES2, IORING_REGISTER_BUFFERS2 */
	static const struct {
		unsigned int op;
		const char *str;
	} rsrc_reg_ops[] = {
		{ 13, "IORING_REGISTER_FILES2" },
		{ 15, "IORING_REGISTER_BUFFERS2" },
	};
	static const struct strval32 rsrc_flags[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "IORING_RSRC_REGISTER_SPARSE") },
		{ ARG_XLAT_UNKNOWN(0x2, "IORING_RSRC_REGISTER_???") },
		{ ARG_XLAT_KNOWN(0xbadc0ded,
				 "IORING_RSRC_REGISTER_SPARSE|0xbadc0dec") },
	};
	static const uint64_t tags[] = { 0x1337, 1, 0xdead, 0xfacefeed,
					 0xbadc0deddadfacedULL, 0 };
	const uint64_t *arg_tags = tail_memdup(tags, sizeof(tags));

	struct io_uring_rsrc_register *bogus_rsrc_reg = tail_alloc(24);
	struct io_uring_rsrc_register *rsrc_reg = tail_alloc(sizeof(*rsrc_reg));
	struct io_uring_rsrc_register *big_rsrc_reg =
		tail_alloc(sizeof(*big_rsrc_reg) + 8);

	fill_memory(big_rsrc_reg, sizeof(*big_rsrc_reg) + 8);

	for (size_t i = 0; i < ARRAY_SIZE(rsrc_reg_ops); i++) {
		sys_io_uring_register(fd_null, rsrc_reg_ops[i].op, 0,
				      0xdeadbeef);
		printf("io_uring_register(%u<%s>, " XLAT_FMT ", NULL, %u)"
		       " = %s\n",
		       fd_null, path_null,
		       XLAT_SEL(rsrc_reg_ops[i].op, rsrc_reg_ops[i].str),
		       0xdeadbeef, errstr);

		struct {
			void *ptr;
			unsigned int sz;
		} ptr_args[] = {
			{ bogus_rsrc_reg, 24 },
			{ bogus_rsrc_reg, 32 },
		};
		for (size_t j = 0; j < ARRAY_SIZE(ptr_args); j++) {
			sys_io_uring_register(fd_null, rsrc_reg_ops[i].op,
					      ptr_args[j].ptr, ptr_args[j].sz);
			printf("io_uring_register(%u<%s>, " XLAT_FMT ", %p, %u)"
			       " = %s\n",
			       fd_null, path_null,
			       XLAT_SEL(rsrc_reg_ops[i].op,
					rsrc_reg_ops[i].str),
			       ptr_args[j].ptr, ptr_args[j].sz, errstr);
		}

		for (size_t j = 0; j < 256; j++) {
			void *endptr = i ? (void *) (arg_iov + ARRAY_SIZE(iov))
					 : (void *) (arg_fds + ARRAY_SIZE(fds));

			rsrc_reg->data = i ? (uintptr_t) arg_iov
					  : (uintptr_t) arg_fds;
			rsrc_reg->nr = i ? ARRAY_SIZE(iov) : ARRAY_SIZE(fds);
			rsrc_reg->tags = (uintptr_t) (arg_tags
						      + ARRAY_SIZE(tags)
						      - rsrc_reg->nr);

			rsrc_reg->data &= ~-!(j & 1);
			rsrc_reg->data |= j & 2 ? BIG_ADDR_MASK : 0;
			rsrc_reg->tags -= !!(j & 64) * sizeof(uint64_t);
			rsrc_reg->tags &= ~-!!(j & 4);
			rsrc_reg->tags |= j & 8 ? BIG_ADDR_MASK : 0;
			rsrc_reg->nr += !!(j & 16);
			rsrc_reg->nr &= ~-!(j & 32);

			rsrc_reg->flags = ARR_ITEM(rsrc_flags, j >> 6).val;
			rsrc_reg->resv2 = j & 128 ? 0xfacecafebeeffeedULL : 0;

			memcpy(big_rsrc_reg, rsrc_reg, sizeof(*rsrc_reg));

			for (size_t k = 1; k < 5; k++) {
				sys_io_uring_register(fd_null,
						      rsrc_reg_ops[i].op,
						      k > 2 ? big_rsrc_reg
						             : rsrc_reg,
						      sizeof(*rsrc_reg)
						      + (k / 2) * 8);
				printf("io_uring_register(%u<%s>, " XLAT_FMT
				       ", {nr=%zu, flags=%s%s",
				       fd_null, path_null,
				       XLAT_SEL(rsrc_reg_ops[i].op,
						rsrc_reg_ops[i].str),
				       j & 32 ? (i ? ARRAY_SIZE(iov)
						   : ARRAY_SIZE(fds))
						+ !!(j & 16) : 0,
				       ARR_ITEM(rsrc_flags, j >> 6).str,
				       j & 128 ? ", resv2=0xfacecafebeeffeed"
					       : "");
				print_rsrc_data(arg_iov, iov, arg_fds, fds,
						i, j, endptr, false);
				print_rsrc_tags(arg_tags, tags, i, j,
						arg_tags + ARRAY_SIZE(tags));
				if (!(k & 1))
					printf(", ???");
				if (k == 3) {
					printf(", /* bytes 32..39 */ \"\\xa0"
					       "\\xa1\\xa2\\xa3\\xa4\\xa5\\xa6"
					       "\\xa7\"");
				}
				printf("}, %zu) = %s\n",
				       sizeof(*rsrc_reg) + (k / 2) * 8, errstr);
			}
		}
	}


	/* IORING_REGISTER_FILES_UPDATE2, IORING_REGISTER_BUFFERS_UPDATE */
	static const struct {
		unsigned int op;
		const char *str;
	} rsrc_upd_ops[] = {
		{ 14, "IORING_REGISTER_FILES_UPDATE2" },
		{ 16, "IORING_REGISTER_BUFFERS_UPDATE" },
	};

	struct io_uring_rsrc_update2 *bogus_rsrc_upd = tail_alloc(24);
	struct io_uring_rsrc_update2 *rsrc_upd = tail_alloc(sizeof(*rsrc_upd));
	struct io_uring_rsrc_update2 *big_rsrc_upd =
		tail_alloc(sizeof(*big_rsrc_upd) + 8);

	fill_memory(big_rsrc_upd, sizeof(*big_rsrc_upd) + 8);

	for (size_t i = 0; i < ARRAY_SIZE(rsrc_upd_ops); i++) {
		sys_io_uring_register(fd_null, rsrc_upd_ops[i].op, 0,
				      0xdeadbeef);
		printf("io_uring_register(%u<%s>, " XLAT_FMT ", NULL, %u)"
		       " = %s\n",
		       fd_null, path_null,
		       XLAT_SEL(rsrc_upd_ops[i].op, rsrc_upd_ops[i].str),
		       0xdeadbeef, errstr);

		struct {
			void *ptr;
			unsigned int sz;
		} ptr_args[] = {
			{ bogus_rsrc_upd, 24 },
			{ bogus_rsrc_upd, 32 },
		};
		for (size_t j = 0; j < ARRAY_SIZE(ptr_args); j++) {
			sys_io_uring_register(fd_null, rsrc_upd_ops[i].op,
					      ptr_args[j].ptr, ptr_args[j].sz);
			printf("io_uring_register(%u<%s>, " XLAT_FMT ", %p, %u)"
			       " = %s\n",
			       fd_null, path_null,
			       XLAT_SEL(rsrc_upd_ops[i].op,
					rsrc_upd_ops[i].str),
			       ptr_args[j].ptr, ptr_args[j].sz, errstr);
		}

		for (size_t j = 0; j < 256; j++) {
			void *endptr = i ? (void *) (arg_iov + ARRAY_SIZE(iov))
					 : (void *) (arg_fds + ARRAY_SIZE(fds));

			rsrc_upd->data = i ? (uintptr_t) arg_iov
					  : (uintptr_t) arg_fds;
			rsrc_upd->nr = i ? ARRAY_SIZE(iov) : ARRAY_SIZE(fds);
			rsrc_upd->tags = (uintptr_t) (arg_tags
						      + ARRAY_SIZE(tags)
						      - rsrc_upd->nr);

			rsrc_upd->data &= ~-!(j & 1);
			rsrc_upd->data |= j & 2 ? BIG_ADDR_MASK : 0;
			rsrc_upd->tags -= !!(j & 64) * sizeof(uint64_t);
			rsrc_upd->tags &= ~-!!(j & 4);
			rsrc_upd->tags |= j & 8 ? BIG_ADDR_MASK : 0;
			rsrc_upd->nr += !!(j & 16);
			rsrc_upd->nr &= ~-!(j & 32);

			rsrc_upd->resv = j & 64 ? 0xbadc0ded : 0;
			rsrc_upd->resv2 = j & 128 ? 0xfacecafe : 0;

			rsrc_upd->offset = j % 3 ? 0 : 0xdeadface;

			memcpy(big_rsrc_upd, rsrc_upd, sizeof(*rsrc_upd));

			for (size_t k = 1; k < 5; k++) {
				sys_io_uring_register(fd_null,
						      rsrc_upd_ops[i].op,
						      k > 2 ? big_rsrc_upd
						             : rsrc_upd,
						      sizeof(*rsrc_upd)
						      + (k / 2) * 8);
				printf("io_uring_register(%u<%s>, " XLAT_FMT
				       ", {offset=%s%s",
				       fd_null, path_null,
				       XLAT_SEL(rsrc_upd_ops[i].op,
						rsrc_upd_ops[i].str),
				       j % 3 ? "0" : "3735943886",
				       j & 64 ? ", resv=0xbadc0ded" : "");
				print_rsrc_data(arg_iov, iov, arg_fds, fds,
						i, j, endptr, true);
				print_rsrc_tags(arg_tags, tags, i, j,
						arg_tags + ARRAY_SIZE(tags));
				printf(", nr=%zu%s",
				       j & 32 ? (i ? ARRAY_SIZE(iov)
						   : ARRAY_SIZE(fds))
						+ !!(j & 16) : 0,
				       j & 128 ? ", resv2=0xfacecafe"
					       : "");
				if (!(k & 1))
					printf(", ???");
				if (k == 3) {
					printf(", /* bytes 32..39 */ \"\\xa0"
					       "\\xa1\\xa2\\xa3\\xa4\\xa5\\xa6"
					       "\\xa7\"");
				}
				printf("}, %zu) = %s\n",
				       sizeof(*rsrc_upd) + (k / 2) * 8, errstr);
			}
		}
	}


	/* IORING_REGISTER_IOWQ_AFF */
	unsigned long aff[] = {
		(unsigned long) 0xbadc0deddadfacedULL,
		(unsigned long) 0xfacefeeddeadbeefULL,
	};
	const unsigned long *arg_aff = tail_memdup(aff, sizeof(aff));
	const unsigned long *arg_aff_end = arg_aff + ARRAY_SIZE(aff);

	sys_io_uring_register(fd_null, 17, NULL, 0xfacefeed);
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x11, "IORING_REGISTER_IOWQ_AFF")
	       ", NULL, 4207869677) = %s\n",
	       fd_null, path_null, errstr);

	sys_io_uring_register(fd_null, 17, arg_aff_end, 0);
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x11, "IORING_REGISTER_IOWQ_AFF") ", [], 0) = %s\n",
	       fd_null, path_null, errstr);

	sys_io_uring_register(fd_null, 17, arg_aff_end, 1);
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x11, "IORING_REGISTER_IOWQ_AFF")
	       ", %p, 1) = %s\n",
	       fd_null, path_null, arg_aff_end, errstr);

	sys_io_uring_register(fd_null, 17, arg_aff + 1, 2);
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x11, "IORING_REGISTER_IOWQ_AFF")
#ifdef WORDS_BIGENDIAN
# if SIZEOF_LONG > 4
	       ", [49 50 51 54 55 57 59 60 61 62 63]" /* face */
# else
	       ", [16 18 19 21 23 25 26 27 28 30 31]" /* dead */
# endif
#else
	       ", [0 1 2 3 5 6 7 9 10 11 12 13 15]" /* beef */
#endif /* WORDS_BIGENDIAN */
	       ", 2) = %s\n",
	       fd_null, path_null, errstr);

	sys_io_uring_register(fd_null, 17, arg_aff + 1, sizeof(aff));
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x11, "IORING_REGISTER_IOWQ_AFF")
	       ", %p, %zu) = %s\n",
	       fd_null, path_null, arg_aff + 1, sizeof(aff), errstr);

	sys_io_uring_register(fd_null, 17, arg_aff, sizeof(aff));
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x11, "IORING_REGISTER_IOWQ_AFF") ", "
	       "[0 2 3 5 6 7 10 11 13 15" /* aced */
	       " 16 17 18 19 20 22 23 25 27 28 30 31" /* dadf */
#if SIZEOF_LONG > 4
	       " 32 34 35 37 38 39 40 42 43" /* 0ded */
	       " 50 51 52 54 55 57 59 60 61 63" /* badc */
	       " 64 65 66 67 69 70 71 73 74 75 76 77 79" /* beef */
	       " 80 82 83 85 87 89 90 91 92 94 95" /* dead feed */
	       " 96 98 99 101 102 103 105 106 107 108 109 110 111"
	       " 113 114 115 118 119 121 123 124 125 126 127"/*face*/
#else
	       " 32 33 34 35 37 38 39 41 42 43 44 45 47" /* beef */
	       " 48 50 51 53 55 57 58 59 60 62 63" /* dead */
#endif
	       "], %zu) = %s\n",
	       fd_null, path_null, sizeof(aff), errstr);


	/* IORING_REGISTER_IOWQ_MAX_WORKERS */
	unsigned int maxw[] = { 0, 1, 0xbedfaced };
	const unsigned int *arg_maxw = tail_memdup(maxw, sizeof(maxw));
	const unsigned int *arg_maxw_end = arg_maxw + ARRAY_SIZE(maxw);

	sys_io_uring_register(fd_null, 19, NULL, 0xfacefeed);
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x13, "IORING_REGISTER_IOWQ_MAX_WORKERS")
	       ", NULL, 4207869677) = %s\n",
	       fd_null, path_null, errstr);

	sys_io_uring_register(fd_null, 19, arg_maxw_end, 0);
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x13, "IORING_REGISTER_IOWQ_MAX_WORKERS")
	       ", [], 0) = %s\n",
	       fd_null, path_null, errstr);

	sys_io_uring_register(fd_null, 19, arg_maxw_end, 1);
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x13, "IORING_REGISTER_IOWQ_MAX_WORKERS")
	       ", %p, 1) = %s\n",
	       fd_null, path_null, arg_maxw_end, errstr);

	sys_io_uring_register(fd_null, 19, arg_maxw_end - 1, 2);
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x13, "IORING_REGISTER_IOWQ_MAX_WORKERS") ", [["
	       XLAT_KNOWN(0, "IO_WQ_BOUND") "]=3202329837, ... /* %p */]"
	       ", 2) = %s\n",
	       fd_null, path_null, arg_maxw_end, errstr);

	sys_io_uring_register(fd_null, 19, arg_maxw, 3);
	printf("io_uring_register(%u<%s>, "
	       XLAT_KNOWN(0x13, "IORING_REGISTER_IOWQ_MAX_WORKERS") ", "
	       "[[" XLAT_KNOWN(0, "IO_WQ_BOUND") "]=0, ["
	       XLAT_KNOWN(1, "IO_WQ_UNBOUND") "]=1, ["
	       XLAT_UNKNOWN(2, "IO_WQ_???") "]=3202329837] => "
#if RETVAL_INJECTED
	       "[[" XLAT_KNOWN(0, "IO_WQ_BOUND") "]=0, ["
	       XLAT_KNOWN(1, "IO_WQ_UNBOUND") "]=1, ["
	       XLAT_UNKNOWN(2, "IO_WQ_???") "]=3202329837]"
#else
	       "%p"
#endif
	       ", 3) = %s\n",
	       fd_null, path_null,
#if !RETVAL_INJECTED
	       arg_maxw,
#endif
	       errstr);


	/* IORING_REGISTER_RING_FDS, IORING_UNREGISTER_RING_FDS */
	static const struct {
		unsigned int op;
		const char *str;
	} ringfd_ops[] = {
		{ 20, "IORING_REGISTER_RING_FDS" },
		{ 21, "IORING_UNREGISTER_RING_FDS" },
	};

	static const size_t ringfd_count = DEFAULT_STRLEN + 1;
	static const uint32_t ringfd_off[] =
		{ -1U, 0, 1, 2, 161803398, 3141592653, -2U };
	TAIL_ALLOC_OBJECT_VAR_ARR(struct io_uring_rsrc_update, ringfds,
				  ringfd_count);

	fill_memory(ringfds, sizeof(*ringfds) * ringfd_count);
	for (size_t i = 0; i < ringfd_count; i++) {
		ringfds[i].offset = ARR_ITEM(ringfd_off, i);
		ringfds[i].resv = i % 2 ? i * 0x1010101 : 0;
		ringfds[i].data = (i % 4 ? 0xbadc0ded00000000ULL : 0)
				  | ARR_ITEM(fds, i);
	}

	for (size_t i = 0; i < ARRAY_SIZE(ringfd_ops); i++) {
		sys_io_uring_register(fd_null, ringfd_ops[i].op, 0,
				      0xdeadbeef);
		printf("io_uring_register(%u<%s>, " XLAT_FMT ", NULL, %u)"
		       " = %s\n",
		       fd_null, path_null,
		       XLAT_SEL(ringfd_ops[i].op, ringfd_ops[i].str),
		       0xdeadbeef, errstr);

		sys_io_uring_register(fd_null, ringfd_ops[i].op,
				      ringfds + ringfd_count, 0);
		printf("io_uring_register(%u<%s>, " XLAT_FMT ", [], 0) = %s\n",
		       fd_null, path_null,
		       XLAT_SEL(ringfd_ops[i].op, ringfd_ops[i].str),
		       errstr);

		sys_io_uring_register(fd_null, ringfd_ops[i].op,
				      ringfds + ringfd_count, 1);
		printf("io_uring_register(%u<%s>, " XLAT_FMT ", %p, 1)"
		       " = %s\n",
		       fd_null, path_null,
		       XLAT_SEL(ringfd_ops[i].op, ringfd_ops[i].str),
		       ringfds + ringfd_count, errstr);

		/* offs:sz: 33-31:32, 33-32:32, 33-32:33, 33-33:33 */
		for (size_t j = 0; j < 4; j++) {
			const size_t offs = (4 - j) / 2;
			const size_t sz = 32 + j / 2;

			sys_io_uring_register(fd_null, ringfd_ops[i].op,
					      ringfds + offs, sz);
			printf("io_uring_register(%u<%s>, " XLAT_FMT ", [",
			       fd_null, path_null,
			       XLAT_SEL(ringfd_ops[i].op, ringfd_ops[i].str));
			for (uint32_t k = offs; k < MIN(ringfd_count,
							DEFAULT_STRLEN + offs);
			     k++) {
				printf("%s{offset=", k != offs ? ", " : "");
				printf(ringfd_ops[i].op == 21 ||
				       k % ARRAY_SIZE(ringfd_off) ? "%u" : "%d",
				       ARR_ITEM(ringfd_off, k));
				if (k % 2)
					printf(", resv=%#x", k * 0x1010101);

				const size_t fdid = k % ARRAY_SIZE(fds);
				if (ringfd_ops[i].op == 20) {
					printf(", data=%d%s%s%s",
					       fds[fdid],
					       paths[fdid] ? "<": "",
					       paths[fdid] ? paths[fdid] : "",
					       paths[fdid] ? ">": "");
				} else {
					if (!((k % ARRAY_SIZE(fds) == 1)
					      && !(k % 4))) {
						printf(", data=%#llx",
						       (k % 4
						        ? 0xbadc0ded00000000ULL
							: 0) | fds[fdid]);
					}
				}
				printf("}");
			}
			if (j != 1)
				printf(", ...");
			if (!(j % 2))
				printf(" /* %p */", ringfds + ringfd_count);
			printf("], %zu) = %s\n", sz, errstr);
		}
	}


	/* IORING_REGISTER_PBUF_RING, IORING_UNREGISTER_PBUF_RING */
	static const struct {
		unsigned int op;
		const char *str;
	} buf_reg_ops[] = {
		{ 22, "IORING_REGISTER_PBUF_RING" },
		{ 23, "IORING_UNREGISTER_PBUF_RING" },
	};
	TAIL_ALLOC_OBJECT_VAR_PTR(struct io_uring_buf_reg, buf_reg);

	for (size_t i = 0; i < ARRAY_SIZE(buf_reg_ops); i++) {
		sys_io_uring_register(fd_null, buf_reg_ops[i].op, 0,
				      0xdeadbeef);
		printf("io_uring_register(%u<%s>, " XLAT_FMT ", NULL, %u)"
		       " = %s\n",
		       fd_null, path_null,
		       XLAT_SEL(buf_reg_ops[i].op, buf_reg_ops[i].str),
		       0xdeadbeef, errstr);

		sys_io_uring_register(fd_null, buf_reg_ops[i].op,
				      buf_reg + 1, 0);
		printf("io_uring_register(%u<%s>, " XLAT_FMT ", %p, 0) = %s\n",
		       fd_null, path_null,
		       XLAT_SEL(buf_reg_ops[i].op, buf_reg_ops[i].str),
		       buf_reg + 1, errstr);

		for (size_t j = 0; j < 256; j++) {
			memset(buf_reg, 0, sizeof(*buf_reg));
			buf_reg->ring_addr = j & 2 ? (uintptr_t) buf_reg : 0;
			buf_reg->ring_entries = j & 4 ? 3141592653 : 0;
			buf_reg->bgid = j & 8 ? 42069 : 0;
			buf_reg->flags = j & 16 ? 31337 : 0;
			buf_reg->resv[0] = j &  32 ? 0xbadc0deddeadfaceULL : 0;
			buf_reg->resv[1] = j &  64 ? 0xdecaffedbeefdeadULL : 0;
			buf_reg->resv[2] = j & 128 ? 0xbadc0dedfacefeedULL : 0;

			sys_io_uring_register(fd_null, buf_reg_ops[i].op,
					      buf_reg, 0x42);
			printf("io_uring_register(%u<%s>, " XLAT_FMT
			       ", {ring_addr=",
			       fd_null, path_null,
			       XLAT_SEL(buf_reg_ops[i].op, buf_reg_ops[i].str));
			if (j & 2)
				printf("%p", buf_reg);
			else
				printf("NULL");
			printf(", ring_entries=%s, bgid=%s, flags=%s",
			       j & 4 ? "3141592653" : "0",
			       j & 8 ? "42069" : "0",
			       j & 16 ? "0x7a69" : "0");
			if (j & 0xe0) {
				printf(", resv=[%s, %s, %s]",
				       j &  32 ? "0xbadc0deddeadface" : "0",
				       j &  64 ? "0xdecaffedbeefdead" : "0",
				       j & 128 ? "0xbadc0dedfacefeed" : "0");
			}
			printf("}, 66) = %s\n", errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
