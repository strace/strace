/*
 * Check how seccomp SECCOMP_SET_MODE_STRICT is decoded.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_seccomp && defined __NR_exit

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char text1[] =
		"seccomp(SECCOMP_SET_MODE_STRICT, 0, NULL) = 0\n";
	static const char text2[] = "+++ exited with 0 +++\n";
	const kernel_ulong_t addr = (kernel_ulong_t) 0xfacefeeddeadbeefULL;
	long rc;

	rc = syscall(__NR_seccomp, -1L, -1L, addr);
	printf("seccomp(%#x /* SECCOMP_SET_MODE_??? */, %u, %#llx)"
	       " = %s\n", -1, -1, (unsigned long long) addr, sprintrc(rc));
	fflush(stdout);

	rc = syscall(__NR_seccomp, 0, 0, 0);
	if (rc) {
		printf("seccomp(SECCOMP_SET_MODE_STRICT, 0, NULL) = %s\n",
		       sprintrc(rc));
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

SKIP_MAIN_UNDEFINED("__NR_seccomp && __NR_exit")

#endif
