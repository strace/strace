/*
 * Check decoding of prctl PR_PAC_RESET_KEYS operation.
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

	rc = syscall(__NR_prctl, PR_PAC_RESET_KEYS, 0x20, 0, 0, 0);
	if (rc >= 0) {
		error_msg_and_fail("prctl(PR_PAC_RESET_KEYS, 0x20) unexpected "
				   "success");
	}
	printf("prctl(PR_PAC_RESET_KEYS, 0x20 /* PR_PAC_??? */, 0, 0, 0)"
	       " = %s\n", sprintrc(rc));

	static const struct {
		kernel_ulong_t val;
		const char *str;
	} arg2[] = {
		{ ARG_STR(0) },
		{ ARG_STR(PR_PAC_APIAKEY) },
		{ ARG_STR(PR_PAC_APIBKEY|PR_PAC_APDAKEY|PR_PAC_APDBKEY|PR_PAC_APGAKEY|0xcae0) },
#if SIZEOF_KERNEL_LONG_T == 8
		{ ARG_ULL_STR(0xbadc0deddadface0) " /* PR_PAC_??? */" }
#else
		{ ARG_STR(0xbadface0) " /* PR_PAC_??? */" },
#endif
	};

	for (size_t i = 0; i < ARRAY_SIZE(arg2); i++) {
		rc = syscall(__NR_prctl, PR_PAC_RESET_KEYS, arg2[i].val,
			     1, 2, 3);
		printf("prctl(PR_PAC_RESET_KEYS, %s, 0x1, 0x2, 0x3) = %s\n",
		       arg2[i].str, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
