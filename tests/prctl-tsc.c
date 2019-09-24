/*
 * Check decoding of prctl PR_GET_TSC/PR_SET_TSC operations.
 *
 * Copyright (c) 2016 JingPiao Chen <chenjingpiao@foxmail.com>
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <linux/prctl.h>

#if defined __NR_prctl && defined PR_GET_TSC && defined PR_SET_TSC

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const kernel_ulong_t bogus_tsc =
		(kernel_ulong_t) 0xdeadc0defacebeefULL;

	TAIL_ALLOC_OBJECT_CONST_PTR(int, tsc);
	long rc;

	rc = syscall(__NR_prctl, PR_SET_TSC, 0);
	printf("prctl(PR_SET_TSC, 0 /* PR_TSC_??? */) = %s\n", sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SET_TSC, bogus_tsc);
	printf("prctl(PR_SET_TSC, %#x /* PR_TSC_??? */) = %s\n",
	       (unsigned int) bogus_tsc, sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SET_TSC, PR_TSC_SIGSEGV);
	printf("prctl(PR_SET_TSC, PR_TSC_SIGSEGV) = %s\n", sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_TSC, NULL);
	printf("prctl(PR_GET_TSC, NULL) = %s\n", sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_TSC, tsc + 1);
	printf("prctl(PR_GET_TSC, %p) = %s\n", tsc + 1, sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_TSC, tsc);
	if (rc)
		printf("prctl(PR_GET_TSC, %p) = %s\n", tsc, sprintrc(rc));
	else
		printf("prctl(PR_GET_TSC, [PR_TSC_SIGSEGV]) = %s\n",
		       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl && PR_GET_TSC && PR_SET_TSC")

#endif
