/*
 * Check decoding of prctl PR_SVE_SET_VL/PR_SVE_GET_VL operations.
 *
 * Copyright (c) 2021-2023 The strace developers.
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
	long rc;

	prctl_marker();

	rc = syscall(__NR_prctl, PR_SVE_SET_VL, 0xf);
	printf("prctl(PR_SVE_SET_VL, %#lx) = %s\n", (unsigned long) 0xf,
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SVE_SET_VL, 0xff);
	printf("prctl(PR_SVE_SET_VL, %#lx) = %s\n", (unsigned long) 0xff,
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SVE_SET_VL, PR_SVE_SET_VL_ONEXEC | 0xff);
	printf("prctl(PR_SVE_SET_VL, PR_SVE_SET_VL_ONEXEC|%#lx) = %s\n",
		   (unsigned long) 0xff, sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SVE_SET_VL, PR_SVE_VL_INHERIT | 0xff);
	printf("prctl(PR_SVE_SET_VL, PR_SVE_VL_INHERIT|%#lx) = %s\n",
		   (unsigned long) 0xff, sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SVE_SET_VL,
				 PR_SVE_SET_VL_ONEXEC | PR_SVE_VL_INHERIT | 0xff);
	printf("prctl(PR_SVE_SET_VL, "
		   "PR_SVE_SET_VL_ONEXEC|PR_SVE_VL_INHERIT|%#lx) = %s\n",
		   (unsigned long) 0xff, sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SVE_GET_VL);
	printf("prctl(PR_SVE_GET_VL) = ");
	if (rc >= 0) {
		printf("%#lx", rc);
		if (rc > 0xffff) {
			printf(" (");
			if (rc & PR_SVE_SET_VL_ONEXEC)
				printf("PR_SVE_SET_VL_ONEXEC");
			if (rc & PR_SVE_VL_INHERIT) {
				printf("%sPR_SVE_VL_INHERIT",
				       rc & PR_SVE_SET_VL_ONEXEC ? "|" : "");
			}
			if (rc & ~0x6ffffU) {
				printf("%s%#lx",
				       rc & 0x60000 ? "|" : "", rc & ~0x6ffffU);
			}
			printf("|%#lx)\n", rc & 0xffffU);
		} else {
			puts("");
		}
	} else {
		puts(sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
