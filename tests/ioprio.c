/*
 * Check decoding of ioprio_get and ioprio_set syscalls.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <asm/unistd.h>

#if defined(__NR_ioprio_get) && defined(__NR_ioprio_set)

# include <stdio.h>
# include <unistd.h>

enum {
	IOPRIO_CLASS_NONE,
	IOPRIO_CLASS_RT,
	IOPRIO_CLASS_BE,
	IOPRIO_CLASS_IDLE
};

# include "xlat.h"
# include "xlat/ioprio_class.h"

void
print_ioprio(unsigned long val)
{
	printf(" (IOPRIO_PRIO_VALUE(");
	printxval(ioprio_class, val >> 13, "IOPRIO_CLASS_???");
	printf(", %d))", (int) (val & 0x1fff));
}

int
main(void)
{
	static const kernel_ulong_t bogus_which =
		(kernel_ulong_t) 0xdeadfacefa57beefULL;
	static const kernel_ulong_t bogus_who =
		(kernel_ulong_t) 0xbadc0dedda7a1057ULL;
	static const kernel_ulong_t bogus_ioprio =
		(kernel_ulong_t) 0xdec0ded1facefeedULL;
	static const char * const bogus_ioprio_str =
		"IOPRIO_PRIO_VALUE(0x7d677 /* IOPRIO_CLASS_??? */, 7917)";

	long rc;

	rc = syscall(__NR_ioprio_get, bogus_which, bogus_who);
	printf("ioprio_get(%#x /* IOPRIO_WHO_??? */, %d) = %s\n",
	       (int) bogus_which, (int) bogus_who, sprintrc(rc));

	rc = syscall(__NR_ioprio_get, 1, 0);
	printf("ioprio_get(IOPRIO_WHO_PROCESS, 0) = %s", sprintrc(rc));

	if (rc >= -1)
		print_ioprio(rc);

	puts("");

	rc = syscall(__NR_ioprio_set, 2, 0, 8191);
	printf("ioprio_set(IOPRIO_WHO_PGRP, 0, "
	       "IOPRIO_PRIO_VALUE(IOPRIO_CLASS_NONE, 8191)) = %s\n",
	       sprintrc(rc));

	rc = syscall(__NR_ioprio_set, bogus_which, bogus_who, bogus_ioprio);
	printf("ioprio_set(%#x /* IOPRIO_WHO_??? */, %d, %s) = %s\n",
	       (int) bogus_which, (int) bogus_who, bogus_ioprio_str,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_ioprio_get && __NR_ioprio_set");

#endif
