/*
 * Force legacy printpath/umovestr using PR_SET_DUMPABLE.
 *
 * Copyright (c) 2017-2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <sys/prctl.h>

#ifdef PR_SET_DUMPABLE

# include <stdio.h>
# include <unistd.h>

# include "test_ucopy.h"

int
main(void)
{
	if (!test_process_vm_readv())
		perror_msg_and_skip("process_vm_readv");

	/*
	 * Clearing dumpable flag disallows process_vm_readv.
	 * If the kernel does not contain commit
	 * 84d77d3f06e7e8dea057d10e8ec77ad71f721be3, then
	 * PTRACE_PEEKDATA remains allowed.
	 */
	if (prctl(PR_SET_DUMPABLE, 0))
		perror_msg_and_skip("PR_SET_DUMPABLE 0");

	if (!test_ptrace_peekdata())
		perror_msg_and_skip("PTRACE_PEEKDATA");

	test_printstrn(DEFAULT_STRLEN);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("PR_SET_DUMPABLE")

#endif
