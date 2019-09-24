/*
 * Check decoding of prctl PR_GET_DUMPABLE/PR_SET_DUMPABLE operations.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <linux/prctl.h>

#if defined __NR_prctl && defined PR_GET_DUMPABLE && defined PR_SET_DUMPABLE \
 && !defined __ia64__

# include <stdio.h>
# include <unistd.h>

static const char *errstr;

static long
prctl(kernel_ulong_t arg1, kernel_ulong_t arg2)
{
	static const kernel_ulong_t bogus_arg =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;
	long rc = syscall(__NR_prctl, arg1, arg2, bogus_arg);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const kernel_ulong_t bogus_dumpable1 =
		(kernel_ulong_t) 0xdeadc0de00000001ULL;
	static const kernel_ulong_t bogus_dumpable2 =
		(kernel_ulong_t) 0xdeadc0defacebeefULL;

	static const char * const args[] = {
		"SUID_DUMP_DISABLE",
		"SUID_DUMP_USER",
		"SUID_DUMP_ROOT",
	};

	unsigned int i;

	prctl(PR_SET_DUMPABLE, 3);
	printf("prctl(PR_SET_DUMPABLE, 0x3 /* SUID_DUMP_??? */) = %s\n",
	       errstr);

	prctl(PR_SET_DUMPABLE, bogus_dumpable1);
	if (bogus_dumpable1 == 1) {
		printf("prctl(PR_SET_DUMPABLE, SUID_DUMP_USER) = %s\n", errstr);
	} else {
		printf("prctl(PR_SET_DUMPABLE, %#llx /* SUID_DUMP_??? */)"
		       " = %s\n",
		       (unsigned long long) bogus_dumpable1, errstr);
	}

	prctl(PR_SET_DUMPABLE, bogus_dumpable2);
	printf("prctl(PR_SET_DUMPABLE, %#llx /* SUID_DUMP_??? */) = %s\n",
	       (unsigned long long) bogus_dumpable2, errstr);

	for (i = 0; i < ARRAY_SIZE(args); ++i) {
		prctl(PR_SET_DUMPABLE, i);
		printf("prctl(PR_SET_DUMPABLE, %s) = %s\n", args[i], errstr);

		long rc = prctl(PR_GET_DUMPABLE, bogus_dumpable2);
		if (rc >= 0 && rc < (long) ARRAY_SIZE(args)) {
			printf("prctl(PR_GET_DUMPABLE) = %s (%s)\n",
			       errstr, args[rc]);
		} else {
			printf("prctl(PR_GET_DUMPABLE) = %s\n", errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl && PR_GET_DUMPABLE && PR_SET_DUMPABLE"
		    " && !__ia64__")

#endif
