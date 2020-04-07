/*
 * Check decoding of prctl PR_GET_PDEATHSIG/PR_SET_PDEATHSIG operations.
 *
 * Copyright (c) 2016 JingPiao Chen <chenjingpiao@foxmail.com>
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <linux/prctl.h>

#if defined __NR_prctl && defined PR_GET_PDEATHSIG && defined PR_SET_PDEATHSIG

# include <stdio.h>
# include <unistd.h>
# include <signal.h>

int
main(void)
{
	static const kernel_ulong_t bogus_signal =
		(kernel_ulong_t) 0xbadc0deddeadfeedULL;

	TAIL_ALLOC_OBJECT_CONST_PTR(int, pdeathsig);
	long rc;

	rc = syscall(__NR_prctl, PR_SET_PDEATHSIG, bogus_signal);
	printf("prctl(PR_SET_PDEATHSIG, %llu) = %s\n",
	       (unsigned long long) bogus_signal, sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SET_PDEATHSIG, SIGINT);
	printf("prctl(PR_SET_PDEATHSIG, SIGINT) = %s\n", sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_PDEATHSIG, NULL);
	printf("prctl(PR_GET_PDEATHSIG, NULL) = %s\n", sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_PDEATHSIG, pdeathsig + 1);
	printf("prctl(PR_GET_PDEATHSIG, %p) = %s\n",
	       pdeathsig + 1, sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_PDEATHSIG, pdeathsig);
	if (rc) {
		printf("prctl(PR_GET_PDEATHSIG, %p) = %s\n",
		       pdeathsig, sprintrc(rc));
	} else {
		printf("prctl(PR_GET_PDEATHSIG, [SIGINT]) = %s\n",
		       sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl && PR_GET_PDEATHSIG && PR_SET_PDEATHSIG")

#endif
