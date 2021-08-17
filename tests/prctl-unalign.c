/*
 * Check decoding of prctl PR_GET_UNALIGN/PR_SET_UNALIGN operations.
 *
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <stdio.h>
#include <unistd.h>
#include <linux/prctl.h>

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, unalign);
	long rc;

	prctl_marker();

	rc = syscall(__NR_prctl, PR_SET_UNALIGN, PR_UNALIGN_NOPRINT | PR_UNALIGN_SIGBUS);
	printf("prctl(PR_SET_UNALIGN, PR_UNALIGN_NOPRINT|PR_UNALIGN_SIGBUS) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SET_UNALIGN, 0xff00);
	printf("prctl(PR_SET_UNALIGN, %#llx /* PR_UNALIGN_??? */) = %s\n",
		   (unsigned long long) 0xff00, sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SET_UNALIGN, PR_UNALIGN_SIGBUS);
	printf("prctl(PR_SET_UNALIGN, PR_UNALIGN_SIGBUS) = %s\n", sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_UNALIGN, unalign + 1);
	printf("prctl(PR_GET_UNALIGN, %p) = %s\n", unalign + 1, sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_UNALIGN, NULL);
	printf("prctl(PR_GET_UNALIGN, NULL) = %s\n", sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_UNALIGN, unalign);
	if (rc)
		printf("prctl(PR_GET_UNALIGN, %p) = %s\n", unalign, sprintrc(rc));
	else
		printf("prctl(PR_GET_UNALIGN, [PR_UNALIGN_SIGBUS]) = %s\n",
			   sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
