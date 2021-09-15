/*
 * Check decoding of io_uring_register syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2021 The strace developers.
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
#include <linux/io_uring.h>

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
		perror_msg_and_fail("open: %s", path_full);

	int fds[] = { fd_full, fd_null };
	const int *arg_fds = tail_memdup(fds, sizeof(fds));


	/* Invalid op */
	static const unsigned int invalid_ops[] = { 0xbadc0dedU, 19 };

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
		       ", [%u<%s>, %u<%s>], %u) = %s\n",
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
	       ", {offset=3735929054, fds=[%u<%s>, %u<%s>]}, %u) = %s\n",
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
	       ", resv2=[%#x, %#x, %#x], ops=[]}, 0) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE),
	       probe->last_op, probe->ops_len, probe->resv,
	       probe->resv2[0], probe->resv2[1], probe->resv2[2], errstr);

	probe->last_op = IORING_OP_READV;
	probe->resv = 0;
	sys_io_uring_register(fd_null, IORING_REGISTER_PROBE, probe, 0);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", {last_op=" XLAT_FMT_U
	       ", ops_len=%hhu, resv2=[%#x, %#x, %#x], ops=[]}, 0) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE),
	       XLAT_ARGS(IORING_OP_READV), probe->ops_len,
	       probe->resv2[0], probe->resv2[1], probe->resv2[2], errstr);

	probe->last_op = IORING_OP_EPOLL_CTL;
	probe->resv2[0] = 0;
	probe->resv2[2] = 0;

	probe->ops[0].op = IORING_OP_NOP;
	probe->ops[0].resv = 0xde;
	probe->ops[0].flags = 0;
	probe->ops[0].resv2 = 0xbeefface;

	probe->ops[1].op = 39;
	probe->ops[1].resv = 0;
	probe->ops[1].flags = IO_URING_OP_SUPPORTED;
	probe->ops[1].resv2 = 0xdeadc0de;

	probe->ops[2].op = 40;
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
	       "{op=40" NRAW(" /* IORING_OP_??? */") ", resv=0xaf, flags="
	       XLAT_FMT "}, {op=254" NRAW(" /* IORING_OP_??? */")
	       ", flags=0xc0de" NRAW(" /* IO_URING_OP_??? */") "}]}, 4) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE),
	       XLAT_ARGS(IORING_OP_EPOLL_CTL), probe->ops_len, probe->resv2[1],
	       XLAT_ARGS(IORING_OP_NOP), XLAT_ARGS(IORING_OP_LINKAT),
	       XLAT_ARGS(IO_URING_OP_SUPPORTED),
	       XLAT_ARGS(IO_URING_OP_SUPPORTED|0xbeee), errstr);

	probe->last_op = 40;
	probe->resv2[1] = 0;
	fill_memory_ex(probe->ops, sizeof(probe->ops[0]) * (DEFAULT_STRLEN + 1),
		    0x40, 0x80);
	sys_io_uring_register(fd_null, IORING_REGISTER_PROBE, probe,
			      DEFAULT_STRLEN + 1);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", {last_op=40"
	       NRAW(" /* IORING_OP_??? */") ", ops_len=%hhu, ops=[",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE),
	       probe->ops_len);
	for (size_t i = 0; i < DEFAULT_STRLEN; i++) {
		printf("%s{op=%u" NRAW(" /* IORING_OP_??? */") ", resv=%#hhx"
		       ", flags=",
		       i ? ", " : "", probe->ops[i].op, probe->ops[i].resv);
#if XLAT_RAW
		printf("%#hx",
		       (typeof(probe->ops[i].flags)) (probe->ops[i].flags));
#else /* !XLAT_RAW */
		if (probe->ops[i].flags & 1) {
			printf(VERB("%#hx /* ") "IO_URING_OP_SUPPORTED|%#hx"
			       VERB(" */"),
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
	printf(", ...]}, %d) = %s\n", DEFAULT_STRLEN + 1, errstr);

	probe->last_op = 0;
	probe->ops_len = 0;
	memset(probe->ops, 0, sizeof(probe->ops[0]) * (DEFAULT_STRLEN + 1));
	sys_io_uring_register(fd_null, IORING_REGISTER_PROBE, probe, 8);
	printf("io_uring_register(%u<%s>, " XLAT_FMT ", %p, 8) = %s\n",
	       fd_null, path_null, XLAT_ARGS(IORING_REGISTER_PROBE), probe,
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
		  "register_op=", ARG_STR(IORING_UNREGISTER_IOWQ_AFF),
		  true },
		{ ARG_STR(IORING_RESTRICTION_REGISTER_OP), true,
		  "register_op=", 19, " /* IORING_REGISTER_??? */", false },
		{ ARG_STR(IORING_RESTRICTION_REGISTER_OP), true,
		  "register_op=", 255, " /* IORING_REGISTER_??? */", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_OP), true,
		  "sqe_op=", ARG_STR(IORING_OP_NOP), true },
		{ ARG_STR(IORING_RESTRICTION_SQE_OP), true,
		  "sqe_op=", ARG_STR(IORING_OP_LINKAT), true },
		{ ARG_STR(IORING_RESTRICTION_SQE_OP), true,
		  "sqe_op=", 40, " /* IORING_OP_??? */", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_OP), true,
		  "sqe_op=", 255, " /* IORING_OP_??? */", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_ALLOWED), true,
		  "sqe_flags=", 0, "", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_ALLOWED), true,
		  "sqe_flags=", 32, "IOSQE_BUFFER_SELECT", true },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_ALLOWED), true,
		  "sqe_flags=", 0xff, "IOSQE_FIXED_FILE|IOSQE_IO_DRAIN"
				      "|IOSQE_IO_LINK|IOSQE_IO_HARDLINK"
				      "|IOSQE_ASYNC|IOSQE_BUFFER_SELECT"
				      "|0xc0",
		  true },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_ALLOWED), true,
		  "sqe_flags=", 192, " /* IOSQE_??? */", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_REQUIRED), true,
		  "sqe_flags=", 0, "", false },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_REQUIRED), true,
		  "sqe_flags=", 1, "IOSQE_FIXED_FILE", true },
		{ ARG_STR(IORING_RESTRICTION_SQE_FLAGS_REQUIRED), true,
		  "sqe_flags=", 96, "IOSQE_BUFFER_SELECT|0x40", true },
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
				       NRAW(restrictions_data[i].op_str));
			}
		} else {
			printf("%#x%s /* op: %#x */",
			       restrictions_data[i].opcode,
			       NRAW(restrictions_data[i].opcode_str),
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
					       NRAW(restrictions_data[i].op_str));
				}
			} else {
				printf("%#x%s /* op: %#x */",
				       restrictions_data[i].opcode,
				       NRAW(restrictions_data[i].opcode_str),
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

	puts("+++ exited with 0 +++");
	return 0;
}
