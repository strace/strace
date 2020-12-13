/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_membarrier

# include <assert.h>
# include <errno.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	assert(syscall(__NR_membarrier, 3, 255, -1) == -1);
	int saved_errno = errno;
	printf("membarrier(0x3 /* MEMBARRIER_CMD_??? */"
	       ", MEMBARRIER_CMD_FLAG_CPU|0xfe, -1) = %s\n",
	       sprintrc(-1));
	if (saved_errno != ENOSYS) {
		const char *text_global;
		const char *text;
		int rc = syscall(__NR_membarrier, 0, 0);

		assert(rc >= 0);

		text_global = rc & 1 ? "MEMBARRIER_CMD_GLOBAL" : "";

		switch (rc & ~1) {
		case 0:
			text = "";
			break;
		case 8:
			text = "MEMBARRIER_CMD_PRIVATE_EXPEDITED";
			break;
		case 8|16:
			text = "MEMBARRIER_CMD_PRIVATE_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED";
			break;

		case 2|4|8|16:
			text = "MEMBARRIER_CMD_GLOBAL_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED|"
			       "MEMBARRIER_CMD_PRIVATE_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED";
			break;

		case 2|4|8|16|32|64:
			text = "MEMBARRIER_CMD_GLOBAL_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED|"
			       "MEMBARRIER_CMD_PRIVATE_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED|"
			       "MEMBARRIER_CMD_PRIVATE_EXPEDITED_SYNC_CORE|"
			       "MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED_SYNC_CORE";
			break;

		case 2|4|8|16|32|64|128|256:
			text = "MEMBARRIER_CMD_GLOBAL_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED|"
			       "MEMBARRIER_CMD_PRIVATE_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED|"
			       "MEMBARRIER_CMD_PRIVATE_EXPEDITED_SYNC_CORE|"
			       "MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED_SYNC_CORE|"
			       "MEMBARRIER_CMD_PRIVATE_EXPEDITED_RSEQ|"
			       "MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED_RSEQ";
			break;

		default:
			error_msg_and_fail("membarrier returned %#x, does"
					   " the test have to be updated?", rc);
		}
		printf("membarrier(MEMBARRIER_CMD_QUERY, 0) = %#x (%s%s%s)\n",
		       rc, text_global, text[0] && text_global[0] ? "|" : "",
		       text);

		rc = syscall(__NR_membarrier, 128, 1, -1);
		printf("membarrier(MEMBARRIER_CMD_PRIVATE_EXPEDITED_RSEQ"
		       ", MEMBARRIER_CMD_FLAG_CPU, -1) = %s\n",
		       sprintrc(rc));

	}
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_membarrier")

#endif
