/*
 * Check decoding of unshare syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#ifdef __NR_unshare

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const kernel_ulong_t bogus_flags =
		(kernel_ulong_t) 0xbadc0ded0000000fULL;

	static struct {
		kernel_ulong_t val;
		const char *str;
	} unshare_flags[] = {
		{ ARG_STR(0) },
		{ 0xdeadcaf5,
			"CLONE_NEWTIME|CLONE_FS|CLONE_SIGHAND|CLONE_THREAD"
			"|CLONE_SYSVSEM|CLONE_NEWCGROUP|CLONE_NEWUTS"
			"|CLONE_NEWIPC|CLONE_NEWUSER|CLONE_NEWNET|0x80a8c075" },
		{ 0x2000000, "CLONE_NEWCGROUP" },
		{ ARG_STR(0x81f8f07f) " /* CLONE_??? */" },
	};

	long rc;
	unsigned int i;

	rc = syscall(__NR_unshare, bogus_flags);
	printf("unshare(%#llx /* CLONE_??? */) = %s\n",
	       (unsigned long long) bogus_flags, sprintrc(rc));

	for (i = 0; i < ARRAY_SIZE(unshare_flags); i++) {
		rc = syscall(__NR_unshare, unshare_flags[i].val);
		printf("unshare(%s) = %s\n",
		       unshare_flags[i].str, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_unshare");

#endif
