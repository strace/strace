/*
 * Check decoding of prctl PR_CAP_AMBIENT operations.
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
#include <linux/capability.h>

int
main(void)
{
	long rc;

	prctl_marker();

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE,
				 CAP_NET_RAW, 0, 0);
	printf("prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, CAP_NET_RAW, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE,
				 CAP_AUDIT_CONTROL, 0, 0);
	printf("prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, CAP_AUDIT_CONTROL, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, 0xff, 0, 0);
	printf("prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, 0xff /* CAP_??? */, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER,
				 CAP_NET_RAW, 0, 0);
	printf("prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER, CAP_NET_RAW, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER,
				 CAP_KILL, 0, 0);
	printf("prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER, CAP_KILL, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER, 0xff, 0, 0);
	printf("prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER, 0xff /* CAP_??? */, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, PR_CAP_AMBIENT_IS_SET,
				 CAP_NET_RAW, 0, 0);
	printf("prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_IS_SET, CAP_NET_RAW, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, PR_CAP_AMBIENT_IS_SET,
				 CAP_AUDIT_CONTROL, 0, 0);
	printf("prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_IS_SET, CAP_AUDIT_CONTROL, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, PR_CAP_AMBIENT_IS_SET, 0xff, 0, 0);
	printf("prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_IS_SET, 0xff /* CAP_??? */, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0);
	printf("prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, 0xff, 0xaaaa, 0, 0);
	printf("prctl(PR_CAP_AMBIENT, 0xff /* PR_CAP_AMBIENT_??? */, 0xaaaa, 0, 0) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAP_AMBIENT, 0xff, 0xaaaa, 0xbbbb, 0xcccc);
	printf("prctl(PR_CAP_AMBIENT, 0xff /* PR_CAP_AMBIENT_??? */, 0xaaaa, 0xbbbb, 0xcccc) = %s\n",
		   sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
