/*
 * Check decoding of prctl operations without arguments and return code parsing:
 * PR_GET_KEEPCAPS, PR_GET_SECCOMP, PR_GET_TIMERSLACK, PR_GET_TIMING,
 * PR_TASK_PERF_EVENTS_DISABLE, and PR_TASK_PERF_EVENTS_ENABLE.
 * Also check decoding of prctl operations that check arguments they do not use
 * for zero equality and don't have any presentable semantics for the arguments
 * they use and the return code: PR_GET_NO_NEW_PRIVS, PR_GET_THP_DISABLE,
 * PR_MPX_DISABLE_MANAGEMENT, PR_MPX_ENABLE_MANAGEMENT, PR_GET_IO_FLUSHER,
 * PR_GET_MEMORY_MERGE, PR_SET_MEMORY_MERGE, and unknown operations.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/prctl.h>

#ifdef INJECT_RETVAL
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif

int
main(int argc, char *argv[])
{
	static const kernel_ulong_t bogus_op_bits =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	static const kernel_ulong_t bogus_arg =
		(kernel_ulong_t) 0xfacefeeddeadbeefULL;
	static const struct strval32 options[] = {
		{  7, "PR_GET_KEEPCAPS" },
		{ 13, "PR_GET_TIMING" },
		{ 21, "PR_GET_SECCOMP" },
		{ 30, "PR_GET_TIMERSLACK" },
		{ 31, "PR_TASK_PERF_EVENTS_DISABLE" },
		{ 32, "PR_TASK_PERF_EVENTS_ENABLE" },
	};
	static const struct strval32 options_checking[] = {
		{ 39, "PR_GET_NO_NEW_PRIVS" },
		{ 42, "PR_GET_THP_DISABLE" },
#ifdef INJECT_RETVAL /* avoid calling non-getter prctls without -einject= */
		{ 43, "PR_MPX_ENABLE_MANAGEMENT" },
		{ 44, "PR_MPX_DISABLE_MANAGEMENT" },
#endif /* INJECT_RETVAL */
		{ 58, "PR_GET_IO_FLUSHER" },
		{ 67, "PR_SET_MEMORY_MERGE" },
		{ 68, "PR_GET_MEMORY_MERGE" },
	};
	static const unsigned int options_unknown[] = {
		0, 17, 18, 48, 49, 74, 75, 76,
		0x41555855, /* "AUXV" - 1 */
		0x41555857, /* "AUXV" + 1 */
		0x53564d40, /* "SVMA" - 1 */
		0x53564d42, /* "SVMA" + 1 */
		0x59616d60, /* "Yama" - 1 */
		0x59616d62, /* "Yama" + 1 */
		0xffffffff,
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, ptr);
	long rc;

	prctl_marker();

#ifdef INJECT_RETVAL
	unsigned long num_skip;
	long inject_retval;
	bool locked = false;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);
	inject_retval = strtol(argv[2], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		if (prctl_marker() != inject_retval)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Have not locked on prctl(-1, -2, -3, -4"
				   ", -5) returning %ld", inject_retval);
#endif /* INJECT_RETVAL */

	for (const struct strval32 *i = options;
	     i < options + ARRAY_SIZE(options); i++) {
		rc = syscall(__NR_prctl, i->val | bogus_op_bits);
		printf("prctl(" XLAT_FMT ") = %s" INJ_STR "\n",
		       XLAT_SEL(i->val, i->str), sprintrc(rc));

		rc = syscall(__NR_prctl, i->val, bogus_arg);
		printf("prctl(" XLAT_FMT ") = %s" INJ_STR "\n",
		       XLAT_SEL(i->val, i->str), sprintrc(rc));

		rc = syscall(__NR_prctl, i->val, 0, bogus_arg);
		printf("prctl(" XLAT_FMT ") = %s" INJ_STR "\n",
		       XLAT_SEL(i->val, i->str), sprintrc(rc));

		rc = syscall(__NR_prctl, i->val, 0, 0, bogus_arg);
		printf("prctl(" XLAT_FMT ") = %s" INJ_STR "\n",
		       XLAT_SEL(i->val, i->str), sprintrc(rc));

		rc = syscall(__NR_prctl, i->val, 0, 0, 0, bogus_arg);
		printf("prctl(" XLAT_FMT ") = %s" INJ_STR "\n",
		       XLAT_SEL(i->val, i->str), sprintrc(rc));

		rc = syscall(__NR_prctl, i->val, bogus_arg, bogus_arg + 1,
			    bogus_arg + 2, bogus_arg + 3);
		printf("prctl(" XLAT_FMT ") = %s" INJ_STR "\n",
		       XLAT_SEL(i->val, i->str), sprintrc(rc));
	}

	for (const struct strval32 *i = options_checking;
	     i < options_checking + ARRAY_SIZE(options_checking); i++) {
		rc = syscall(__NR_prctl, i->val | bogus_op_bits, 0, 0, 0, 0);
		printf("prctl(" XLAT_FMT ", 0, 0, 0, 0) = %s" INJ_STR "\n",
		       XLAT_SEL(i->val, i->str), sprintrc(rc));

		rc = syscall(__NR_prctl, i->val, bogus_arg, 0, 0, 0);
		printf("prctl(" XLAT_FMT ", %#llx, 0, 0, 0) = %s" INJ_STR "\n",
		       XLAT_SEL(i->val, i->str), (unsigned long long) bogus_arg,
		       sprintrc(rc));

		rc = syscall(__NR_prctl, i->val, 0, bogus_arg, 0, 0);
		printf("prctl(" XLAT_FMT ", 0, %#llx, 0, 0) = %s" INJ_STR "\n",
		       XLAT_SEL(i->val, i->str), (unsigned long long) bogus_arg,
		       sprintrc(rc));

		rc = syscall(__NR_prctl, i->val, 0, 0, bogus_arg, 0);
		printf("prctl(" XLAT_FMT ", 0, 0, %#llx, 0) = %s" INJ_STR "\n",
		       XLAT_SEL(i->val, i->str), (unsigned long long) bogus_arg,
		       sprintrc(rc));

		rc = syscall(__NR_prctl, i->val, 0, 0, 0, bogus_arg);
		printf("prctl(" XLAT_FMT ", 0, 0, 0, %#llx) = %s" INJ_STR "\n",
		       XLAT_SEL(i->val, i->str), (unsigned long long) bogus_arg,
		       sprintrc(rc));

		rc = syscall(__NR_prctl, i->val, bogus_arg, bogus_arg + 1,
			     bogus_arg + 2, bogus_arg + 3);
		printf("prctl(" XLAT_FMT ", %#llx, %#llx, %#llx, %#llx) = %s"
		       INJ_STR "\n",
		       XLAT_SEL(i->val, i->str),
		       (unsigned long long) bogus_arg,
		       (unsigned long long) bogus_arg + 1,
		       (unsigned long long) bogus_arg + 2,
		       (unsigned long long) bogus_arg + 3,
		       sprintrc(rc));
	}

	for (const unsigned int *i = options_unknown;
	     i < options_unknown + ARRAY_SIZE(options_unknown); i++) {
		rc = syscall(__NR_prctl, *i | bogus_op_bits, 0, 0, 0, 0);
		printf("prctl(%#x" NRAW(" /* PR_??? */") ", 0, 0, 0, 0) = %s"
		       INJ_STR "\n",
		       *i, sprintrc(rc));

		rc = syscall(__NR_prctl, *i, bogus_arg, 0, 0, 0);
		printf("prctl(%#x" NRAW(" /* PR_??? */")
		       ", %#llx, 0, 0, 0) = %s" INJ_STR "\n",
		       *i, (unsigned long long) bogus_arg, sprintrc(rc));

		rc = syscall(__NR_prctl, *i, 0, bogus_arg, 0, 0);
		printf("prctl(%#x" NRAW(" /* PR_??? */")
		       ", 0, %#llx, 0, 0) = %s" INJ_STR "\n",
		       *i, (unsigned long long) bogus_arg, sprintrc(rc));

		rc = syscall(__NR_prctl, *i, 0, 0, bogus_arg, 0);
		printf("prctl(%#x" NRAW(" /* PR_??? */")
		       ", 0, 0, %#llx, 0) = %s" INJ_STR "\n",
		       *i, (unsigned long long) bogus_arg, sprintrc(rc));

		rc = syscall(__NR_prctl, *i, 0, 0, 0, bogus_arg);
		printf("prctl(%#x" NRAW(" /* PR_??? */")
		       ", 0, 0, 0, %#llx) = %s" INJ_STR "\n",
		       *i, (unsigned long long) bogus_arg, sprintrc(rc));

		rc = syscall(__NR_prctl, *i, bogus_arg, bogus_arg + 1,
			     bogus_arg + 2, bogus_arg + 3);
		printf("prctl(%#x" NRAW(" /* PR_??? */")
		       ", %#llx, %#llx, %#llx, %#llx) = %s" INJ_STR "\n",
		       *i, (unsigned long long) bogus_arg,
		       (unsigned long long) bogus_arg + 1,
		       (unsigned long long) bogus_arg + 2,
		       (unsigned long long) bogus_arg + 3,
		       sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
