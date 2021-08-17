/*
 * Check decoding of prctl PR_CAPBSET_READ/PR_CAPBSET_DROP operations.
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

	rc = syscall(__NR_prctl, PR_CAPBSET_READ, CAP_AUDIT_CONTROL);
	printf("prctl(PR_CAPBSET_READ, CAP_AUDIT_CONTROL) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAPBSET_READ, CAP_NET_RAW);
	printf("prctl(PR_CAPBSET_READ, CAP_NET_RAW) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAPBSET_READ, 0xff);
	printf("prctl(PR_CAPBSET_READ, 0xff /* CAP_??? */) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAPBSET_DROP, CAP_AUDIT_CONTROL);
	printf("prctl(PR_CAPBSET_DROP, CAP_AUDIT_CONTROL) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAPBSET_DROP, CAP_NET_RAW);
	printf("prctl(PR_CAPBSET_DROP, CAP_NET_RAW) = %s\n",
		   sprintrc(rc));

	rc = syscall(__NR_prctl, PR_CAPBSET_DROP, 0xff);
	printf("prctl(PR_CAPBSET_DROP, 0xff /* CAP_??? */) = %s\n",
		   sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
