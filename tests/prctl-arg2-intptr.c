/*
 * Check decoding of prctl operations which use arg2 as a pointer to an integer
 * value: PR_GET_CHILD_SUBREAPER, PR_GET_ENDIAN, PR_GET_FPEMU, and PR_GET_FPEXC.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/prctl.h>

static const char *errstr;

static long
prctl(kernel_ulong_t arg1, kernel_ulong_t arg2)
{
	static const kernel_ulong_t bogus_arg =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;
	long rc = syscall(__NR_prctl, arg1, arg2, bogus_arg);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const kernel_ulong_t bogus_addr1 =
		(kernel_ulong_t) 0x1e55c0de00000000ULL;
	static const kernel_ulong_t bogus_addr2 =
		(kernel_ulong_t) 0xfffffffffffffffdULL;
	static const kernel_ulong_t bogus_op_bits =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	static const struct {
		kernel_ulong_t val;
		const char *str;
	} options[] = {
		{ 37, "PR_GET_CHILD_SUBREAPER" },
		{ 19, "PR_GET_ENDIAN" },
		{  9, "PR_GET_FPEMU" },
		{ 11, "PR_GET_FPEXC" },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, ptr);
	long rc;

	prctl_marker();

	for (unsigned int i = 0; i < ARRAY_SIZE(options); ++i) {
		prctl(options[i].val | bogus_op_bits, 0);
		printf("prctl(%s, NULL) = %s\n", options[i].str, errstr);

		if (bogus_addr1) {
			prctl(options[i].val | bogus_op_bits, bogus_addr1);
			printf("prctl(%s, %#llx) = %s\n", options[i].str,
			       (unsigned long long) bogus_addr1, errstr);
		}

		prctl(options[i].val | bogus_op_bits, bogus_addr2);
		printf("prctl(%s, %#llx) = %s\n", options[i].str,
		       (unsigned long long) bogus_addr2, errstr);

		prctl(options[i].val | bogus_op_bits, (uintptr_t) (ptr + 1));
		printf("prctl(%s, %p) = %s\n", options[i].str,
		       ptr + 1, errstr);

		rc = prctl(options[i].val | bogus_op_bits, (uintptr_t) ptr);
		if (!rc) {
			printf("prctl(%s, [%u]) = %s\n",
			       options[i].str, *ptr, errstr);
		} else {
			printf("prctl(%s, %p) = %s\n",
			       options[i].str, ptr, errstr);
		}

		if (F8ILL_KULONG_SUPPORTED) {
			kernel_ulong_t bogus_addr3 = f8ill_ptr_to_kulong(ptr);
			prctl(options[i].val | bogus_op_bits, bogus_addr3);
			printf("prctl(%s, %#llx) = %s\n", options[i].str,
			       (unsigned long long) bogus_addr3, errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
