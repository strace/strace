/*
 * Check decoding of prctl PR_SET_SYSCALL_USER_DISPATCH operation.
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

	rc = syscall(__NR_prctl, PR_SET_SYSCALL_USER_DISPATCH,
		     PR_SYS_DISPATCH_ON,
		     (kernel_ulong_t) 0xdeadbeeffacefeedULL, 0, 0);
	printf("prctl(PR_SET_SYSCALL_USER_DISPATCH, PR_SYS_DISPATCH_ON, %#llx"
	       ", 0, NULL) = %s\n",
	       (unsigned long long) (kernel_ulong_t) 0xdeadbeeffacefeedULL,
	       sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SET_SYSCALL_USER_DISPATCH,
		     PR_SYS_DISPATCH_OFF | F8ILL_KULONG_MASK, 1, 1, 1);
	printf("prctl(PR_SET_SYSCALL_USER_DISPATCH, %s, 0x1, 0x1, 0x1) = %s\n",
	       F8ILL_KULONG_MASK
		? "0xffffffff00000000 /* PR_SYS_DISPATCH_??? */"
		: "PR_SYS_DISPATCH_OFF",
	       sprintrc(rc));

	rc = syscall(__NR_prctl, PR_SET_SYSCALL_USER_DISPATCH,
		     PR_SYS_DISPATCH_OFF, 0,
		     (kernel_ulong_t) 0xdeadbeeffacefeeeULL,
		     (kernel_ulong_t) 0xdeadbeeffacefeefULL);
	printf("prctl(PR_SET_SYSCALL_USER_DISPATCH, PR_SYS_DISPATCH_OFF, 0"
	       ", %#llx, %#llx) = %s\n",
	       (unsigned long long) (kernel_ulong_t) 0xdeadbeeffacefeeeULL,
	       (unsigned long long) (kernel_ulong_t) 0xdeadbeeffacefeefULL,
	       sprintrc(rc));

	const char dummy = 'a';
	rc = syscall(__NR_prctl, PR_SET_SYSCALL_USER_DISPATCH, 2, 1, 1, &dummy);
	printf("prctl(PR_SET_SYSCALL_USER_DISPATCH"
	       ", 0x2 /* PR_SYS_DISPATCH_??? */, 0x1, 0x1, %p) = %s\n",
	       &dummy, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
