/*
 * Check decoding of landlock_restrict_self syscall.
 *
 * Copyright (c) 2021-2024 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

#ifndef RULESET_FD
# define RULESET_FD 7
#endif
#ifndef RULESET_FD_STR
# define RULESET_FD_STR "7"
#endif

static const char *errstr;

static long
sys_landlock_restrict_self(int ruleset_fd, unsigned int flags)
{
	static const kernel_ulong_t fill =
		(kernel_ulong_t) 0xd1efaced00000000ULL;
	kernel_ulong_t arg1 = fill | (unsigned int) ruleset_fd;
	kernel_ulong_t arg2 = fill | flags;
	kernel_ulong_t arg3 = fill | 0xcaffeedc;
	kernel_ulong_t arg4 = fill | 0xbadceded;
	kernel_ulong_t arg5 = fill | 0xdecaffed;
	kernel_ulong_t arg6 = fill | 0xdeefaced;

	long rc = syscall(__NR_landlock_restrict_self,
			  arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	/* Valid attr ptr */
	static const struct strval64 ruleset_fd_vals[] = {
		{ ARG_STR(-1) },
		{ ARG_STR(9409) },
		{ RULESET_FD, RULESET_FD_STR },
	};
	static const struct strival32 flags_vals[] = {
		{ ARG_STR(0) },
		{ ARG_STR(0x1) },
		{ ARG_STR(0xffffffff) },
	};
	for (size_t i = 0; i < ARRAY_SIZE(ruleset_fd_vals); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(flags_vals); j++) {
			sys_landlock_restrict_self(ruleset_fd_vals[i].val,
						   flags_vals[j].val);
			printf("landlock_restrict_self(%s, %s) = %s\n",
			       ruleset_fd_vals[i].str, flags_vals[j].str,
			       errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
