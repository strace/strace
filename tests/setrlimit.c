/*
 * Check decoding of setrlimit syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_setrlimit

# include "xgetrlimit.c"

int
main(void)
{
	kernel_ulong_t *const rlimit = tail_alloc(sizeof(*rlimit) * 2);
	const struct xlat *xlat;

	for (xlat = resources; xlat->str; ++xlat) {
		unsigned long res = 0xfacefeed00000000ULL | xlat->val;
		long rc = syscall(__NR_setrlimit, res, 0);
		printf("setrlimit(%s, NULL) = %s\n", xlat->str, sprintrc(rc));

		struct rlimit libc_rlim = {};
		if (getrlimit((int) res, &libc_rlim))
			continue;
		rlimit[0] = libc_rlim.rlim_cur;
		rlimit[1] = libc_rlim.rlim_max;

		rc = syscall(__NR_setrlimit, res, rlimit);
		const char *errstr = sprintrc(rc);
		printf("setrlimit(%s, {rlim_cur=%s, rlim_max=%s}) = %s\n",
		       xlat->str,
		       sprint_rlim(rlimit[0]), sprint_rlim(rlimit[1]),
		       errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_setrlimit")

#endif
