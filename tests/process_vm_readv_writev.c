/*
 * Check decoding of process_vm_readv/process_vm_writev syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/uio.h>
#include "pidns.h"

#if OP_WR
# define in_iovec  rmt_iovec
# define out_iovec lcl_iovec
# define in_iov    rmt_iov
# define out_iov   lcl_iov
#else
# define in_iovec  lcl_iovec
# define out_iovec rmt_iovec
# define in_iov    lcl_iov
# define out_iov   rmt_iov
#endif

typedef void (*iov_print_fn)(const struct iovec *, const void *, long);

enum { MAX_SEGM_COUNT = 2, MAX_STR_LEN = 5 };

struct print_iov_arg {
	uint32_t count;
	uint32_t valid    :1,
		 string   :1,
		 addr_term:1,
		 check_rc :1;
	uint32_t str_segms;
	uint8_t  str_base[MAX_SEGM_COUNT];
	uint8_t  str_size[MAX_SEGM_COUNT];
};

static void
print_iov(const struct iovec *iov, const void *arg_ptr, long rc)
{
	const struct print_iov_arg *arg = arg_ptr;
	uint32_t num_segm = 0;
	uint64_t segm_offs = 0;

	if (!arg || !arg->valid) {
		if (iov)
			printf("%p", iov);
		else
			printf("NULL");

		return;
	}

	printf("[");

	for (uint32_t i = 0; i < arg->count; ++i) {
		if (i)
			printf(", ");

		if (i >= MAX_STR_LEN) {
			printf("...");
			break;
		}

		printf("{iov_base=");
		if (arg->string && (!arg->check_rc || (rc != -1))) {
			uint64_t str_left = iov[i].iov_len;
			uint64_t pr_count = 0;

			printf("\"");

			while (str_left--) {
				static const char oct_str[] = "01234567";
				uint8_t c = arg->str_base[num_segm] + segm_offs;

				if ((num_segm >= arg->str_segms) ||
				    (num_segm >= MAX_SEGM_COUNT))
					error_msg_and_fail("print_iov: segment "
							   "count overrun");

				if (pr_count++ < MAX_STR_LEN)
					printf("\\%.1s%.1s%d",
					       (c >> 6) ?
					       oct_str + (c >> 6) : "",
					       (c >> 3) ?
					       oct_str + ((c >> 3) & 7) : "",
					       c & 7);

				segm_offs++;

				if (segm_offs >= arg->str_size[num_segm]) {
					num_segm++;
					segm_offs = 0;
				}
			}

			printf("\"");

			if (pr_count > MAX_STR_LEN)
				printf("...");
		} else {
			if (iov[i].iov_base)
				printf("%p", iov[i].iov_base);
			else
				printf("NULL");
		}

		printf(", iov_len=%zu}", iov[i].iov_len);
	}

	if (arg->addr_term)
		printf(", ... /* %p */", iov + arg->count);

	printf("]");
}

static void
do_call(kernel_ulong_t pid, enum pid_type pid_type,
	kernel_ulong_t local_iov, const char *local_arg,
	kernel_ulong_t liovcnt,
	kernel_ulong_t remote_iov, const char *remote_arg,
	kernel_ulong_t riovcnt,
	kernel_ulong_t flags, iov_print_fn pr_iov)
{
	long rc;
	const char *errstr;

	rc = syscall(OP_NR, pid, local_iov, liovcnt, remote_iov, riovcnt,
		flags);
	errstr = sprintrc(rc);

	pidns_print_leader();
	printf("%s(%d%s, ", OP_STR, (int) pid, pidns_pid2str(pid_type));

	if (pr_iov)
		pr_iov((const struct iovec *) (uintptr_t) local_iov, local_arg,
			rc);
	else
		printf("%s", local_arg);

	printf(", %lu, ", (unsigned long) liovcnt);

	if (pr_iov)
		pr_iov((const struct iovec *) (uintptr_t) remote_iov,
		       remote_arg, rc);
	else
		printf("%s", remote_arg);

	printf(", %lu, %lu) = %s\n", (unsigned long) riovcnt,
		(unsigned long) flags, errstr);
}

static kernel_ulong_t
ptr_cast(void *ptr)
{
	return (kernel_ulong_t) (uintptr_t) ptr;
}

int
main(void)
{
	PIDNS_TEST_INIT;

	enum {
		SIZE_11 = 2,
		SIZE_12 = 3,
		SIZE_13 = 4,
		SIZE_1 = SIZE_11 + SIZE_12 + SIZE_13,
		SIZE_21 = 5,
		SIZE_22 = 6,
		SIZE_23 = 7,
		SIZE_2 = SIZE_21 + SIZE_22 + SIZE_23,
	};

	enum {
		SEGM1_BASE = 0x80,
		SEGM2_BASE = 0xA0,
	};

	static const kernel_ulong_t bogus_pid =
		(kernel_ulong_t) 0xbadfaceddeadca57ULL;
	static const kernel_ulong_t bogus_iovcnt1 =
		(kernel_ulong_t) 0xdec0ded1defaced2ULL;
	static const kernel_ulong_t bogus_iovcnt2 =
		(kernel_ulong_t) 0xdec0ded3defaced4ULL;
	static const kernel_ulong_t bogus_flags =
		(kernel_ulong_t) 0xdeadc0deda7adeadULL;

	pid_t my_pid = getpid();
	char *data1_out = tail_alloc(SIZE_1);
	char *data2_out = tail_alloc(SIZE_2);
	char *data1_in  = tail_alloc(SIZE_2);
	char *data2_in  = tail_alloc(SIZE_1);

	struct iovec bogus_iovec[] = {
		{ data1_out + SIZE_1, (size_t) 0xdeadfaceca57beefULL },
		{ data1_in  + SIZE_2, (size_t) 0xbadc0dedda7adeadULL },
		{ data2_out + SIZE_2, (size_t) 0xf157facedec0ded1ULL },
		{ data2_in  + SIZE_1, (size_t) 0xdefaced2bea7be57ULL },
	};

	struct iovec out_iovec[] = {
		{ data1_out,  SIZE_11 },
		{ data1_out + SIZE_11,  SIZE_12 },
		{ data1_out + SIZE_11 + SIZE_12,  SIZE_13 },
		{ data2_out,  SIZE_21 },
		{ data2_out + SIZE_21,  SIZE_22 },
		{ data2_out + SIZE_21 + SIZE_22,  SIZE_23 },
	};
	struct iovec in_iovec[] = {
		{ data1_in,  SIZE_23 },
		{ data1_in + SIZE_23,  SIZE_22 },
		{ data1_in + SIZE_23 + SIZE_22,  SIZE_21 },
		{ data2_in,  SIZE_13 },
		{ data2_in + SIZE_13,  SIZE_12 },
		{ data2_in + SIZE_13 + SIZE_12,  SIZE_11 },
	};

	struct iovec *bogus_iov = tail_memdup(bogus_iovec, sizeof(bogus_iovec));
	struct iovec *lcl_iov   = tail_memdup(lcl_iovec,   sizeof(lcl_iovec));
	struct iovec *rmt_iov   = tail_memdup(rmt_iovec,   sizeof(rmt_iovec));

	struct print_iov_arg bogus_arg   = { ARRAY_SIZE(bogus_iovec), 1 };
	struct print_iov_arg lcl_arg     = { ARRAY_SIZE(lcl_iovec), 1, 1, 0, 0,
		2, {SEGM1_BASE, SEGM2_BASE}, {SIZE_1, SIZE_2} };
	struct print_iov_arg rmt_arg     = { ARRAY_SIZE(rmt_iovec), 1 };

	struct print_iov_arg bogus_arg_cut = {
		ARRAY_SIZE(bogus_iovec) - 2, 1, 0, 1
	};
	struct print_iov_arg lcl_arg_cut = {
		ARRAY_SIZE(lcl_iovec) - 2, 1, 1, 1, 0, 2,
		{ SEGM1_BASE + SIZE_11 + SIZE_12, SEGM2_BASE },
		{SIZE_13, SIZE_2}
	};
	struct print_iov_arg rmt_arg_cut = { ARRAY_SIZE(rmt_iovec) - 2, 1 };


	fill_memory_ex(data1_out, SIZE_1, SEGM1_BASE, SIZE_1);
	fill_memory_ex(data2_out, SIZE_2, SEGM2_BASE, SIZE_2);


	do_call(bogus_pid, PT_NONE, (kernel_ulong_t) (uintptr_t) ARG_STR(NULL),
		bogus_iovcnt1, (kernel_ulong_t) (uintptr_t) ARG_STR(NULL),
		bogus_iovcnt2, bogus_flags, NULL);

	do_call(my_pid, PT_TGID, ptr_cast(bogus_iov + ARRAY_SIZE(bogus_iovec)),
		"[]", 0, ptr_cast(in_iov + ARRAY_SIZE(in_iovec)), "[]",
		0, 0, NULL);
	do_call(my_pid, PT_TGID, ptr_cast(bogus_iov + ARRAY_SIZE(bogus_iovec)),
		NULL, bogus_iovcnt1, ptr_cast(in_iov + ARRAY_SIZE(in_iovec)),
		NULL, bogus_iovcnt2, 0, print_iov);

	do_call(my_pid, PT_TGID, ptr_cast(bogus_iov), (char *) &bogus_arg,
		ARRAY_SIZE(bogus_iovec), ptr_cast(rmt_iov + 2),
		(char *) &rmt_arg_cut, ARRAY_SIZE(rmt_iovec) - 2, 0, print_iov);

#if !OP_WR
	lcl_arg.check_rc = 1;
	lcl_arg_cut.check_rc = 1;
#endif

	do_call(my_pid, PT_TGID, ptr_cast(lcl_iov + 2), (char *) &lcl_arg_cut,
		ARRAY_SIZE(lcl_iovec) - 1, ptr_cast(bogus_iov + 2),
		(char *) &bogus_arg_cut, ARRAY_SIZE(bogus_iovec) - 1, 0,
		print_iov);

	lcl_arg_cut.addr_term = 0;

	rmt_arg_cut.addr_term = 1;
	rmt_arg_cut.count = 5;

	do_call(my_pid, PT_TGID, ptr_cast(lcl_iov + 2), (char *) &lcl_arg_cut,
		ARRAY_SIZE(lcl_iovec) - 2, ptr_cast(rmt_iov + 1),
		(char *) &rmt_arg_cut, ARRAY_SIZE(rmt_iovec), 0, print_iov);

	/* Correct call */
	do_call(my_pid, PT_TGID, ptr_cast(lcl_iov), (char *) &lcl_arg,
		ARRAY_SIZE(lcl_iovec), ptr_cast(rmt_iov), (char *) &rmt_arg,
		ARRAY_SIZE(rmt_iovec), 0, print_iov);

	pidns_print_leader();
	puts("+++ exited with 0 +++");

	return 0;
}
