/*
 * Check decoding of prctl PR_MCE_KILL/PR_MCE_KILL_GET operations.
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
	long rc;

	prctl_marker();

	rc = syscall(__NR_prctl, PR_MCE_KILL, PR_MCE_KILL_CLEAR, 0, 0, 0);
	printf("prctl(PR_MCE_KILL, PR_MCE_KILL_CLEAR, 0, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_MCE_KILL, PR_MCE_KILL_SET, PR_MCE_KILL_EARLY, 0, 0);
	printf("prctl(PR_MCE_KILL, PR_MCE_KILL_SET, PR_MCE_KILL_EARLY, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_MCE_KILL, PR_MCE_KILL_SET, PR_MCE_KILL_LATE, 0, 0);
	printf("prctl(PR_MCE_KILL, PR_MCE_KILL_SET, PR_MCE_KILL_LATE, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_MCE_KILL, PR_MCE_KILL_SET, 0xff, 0, 0);
	printf("prctl(PR_MCE_KILL, PR_MCE_KILL_SET, 0xff /* PR_MCE_KILL_??? */, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_MCE_KILL, 0xaaaa, 0xbbbb, 0xcccc, 0xdddd);
	printf("prctl(PR_MCE_KILL, 0xaaaa /* PR_MCE_KILL_??? */, 0xbbbb, 0xcccc, 0xdddd) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_MCE_KILL, PR_MCE_KILL_SET, PR_MCE_KILL_DEFAULT, 0, 0);
	printf("prctl(PR_MCE_KILL, PR_MCE_KILL_SET, PR_MCE_KILL_DEFAULT, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_MCE_KILL_GET, 0, 0, 0, 0);
	if (rc)
		printf("prctl(PR_MCE_KILL_GET, 0, 0, 0, 0) = %s (PR_MCE_KILL_DEFAULT)\n",
			   sprintrc(rc));
	else
		printf("prctl(PR_MCE_KILL_GET, 0, 0, 0, 0) = %s\n",
			   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_MCE_KILL_GET, 0xaaaa, 0xbbbb, 0xcccc, 0xdddd);
	printf("prctl(PR_MCE_KILL_GET, 0xaaaa, 0xbbbb, 0xcccc, 0xdddd) = %s\n",
		   sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
