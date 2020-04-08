/*
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <sys/prctl.h>
#include "scno.h"

#if defined PR_SET_SECCOMP && defined __NR_exit

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char text1[] =
		"prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT) = 0\n";
	static const char text2[] = "+++ exited with 0 +++\n";

	int rc = prctl(PR_SET_SECCOMP, -1L, 1, 2, 3);
	printf("prctl(PR_SET_SECCOMP, %#lx /* SECCOMP_MODE_??? */, 0x1, 0x2, 0x3)"
	       " = %d %s (%m)\n", -1L, rc, errno2name());
	fflush(stdout);

	rc = prctl(PR_SET_SECCOMP, 1);
	if (rc) {
		printf("prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT)"
		       " = %d %s (%m)\n", rc, errno2name());
		fflush(stdout);
		rc = 0;
	} else {
		/*
		 * If kernel implementation of SECCOMP_MODE_STRICT is buggy,
		 * the following syscall will result to SIGKILL.
		 */
		rc = write(1, text1, LENGTH_OF(text1)) != LENGTH_OF(text1);
	}

	rc += write(1, text2, LENGTH_OF(text2)) != LENGTH_OF(text2);
	return !!syscall(__NR_exit, rc);
}

#else

SKIP_MAIN_UNDEFINED("PR_SET_SECCOMP && __NR_exit")

#endif
