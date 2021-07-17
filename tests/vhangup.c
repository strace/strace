/*
 * Check decoding of vhangup syscall.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	if (setsid() == -1)
		perror_msg_and_skip("setsid");

	long rc = syscall(__NR_vhangup);

	/*
	 * On setsid() success, the new session has no controlling terminal,
	 * therefore a subsequent vhangup() has nothing to hangup.
	 *
	 * The system call, however, returns 0 iff the calling process
	 * has CAP_SYS_TTY_CONFIG capability.
	 */
	printf("vhangup() = %s\n", sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
