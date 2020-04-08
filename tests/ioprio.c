/*
 * Check decoding of ioprio_get and ioprio_set syscalls.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

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

int
main(void)
{
	static const kernel_ulong_t bogus_which =
		(kernel_ulong_t) 0xdeadfacefa57beefULL;
	static const kernel_ulong_t bogus_who =
		(kernel_ulong_t) 0xbadc0dedda7a1057ULL;
	static const kernel_ulong_t bogus_ioprio =
		(kernel_ulong_t) 0xdec0ded1facefeedULL;
# if !XLAT_RAW
	static const char * const bogus_ioprio_str =
		"IOPRIO_PRIO_VALUE(0x7d677 /* IOPRIO_CLASS_??? */, 7917)";
# endif

	long rc;
	const char *errstr;

	rc = syscall(__NR_ioprio_get, bogus_which, bogus_who);
	errstr = sprintrc(rc);
# if XLAT_RAW
	printf("ioprio_get(%#x, %d) = %s\n",
	       (int) bogus_which, (int) bogus_who, errstr);
# else /* XLAT_ABBREV || XLAT_VERBOSE */
	printf("ioprio_get(%#x /* IOPRIO_WHO_??? */, %d) = %s\n",
	       (int) bogus_which, (int) bogus_who, errstr);
# endif

	rc = syscall(__NR_ioprio_get, 1, 0);
	errstr = sprintrc(rc);
# if XLAT_RAW
	printf("ioprio_get(0x1, 0) = %s\n", errstr);
# else /* XLAT_ABBREV */
#  if XLAT_VERBOSE
	printf("ioprio_get(0x1 /* IOPRIO_WHO_PROCESS */, 0) = %s", errstr);
#  else
	printf("ioprio_get(IOPRIO_WHO_PROCESS, 0) = %s", errstr);
#  endif
	if (rc >= 0) {
		printf(" (IOPRIO_PRIO_VALUE(");
		printxval(ioprio_class, (unsigned int) rc >> 13,
			  "IOPRIO_CLASS_???");
		printf(", %u))", (unsigned int) rc & 0x1fff);
	}
	puts("");
# endif

	rc = syscall(__NR_ioprio_set, 2, 0, 8191);
	errstr = sprintrc(rc);
# if XLAT_RAW
	printf("ioprio_set(%#x, 0, 8191) = %s\n", 2, errstr);
# elif XLAT_VERBOSE
	printf("ioprio_set(%#x /* IOPRIO_WHO_PGRP */, 0, 8191"
	       " /* IOPRIO_PRIO_VALUE(0 /* IOPRIO_CLASS_NONE */, 8191) */)"
	       " = %s\n",
	       2, errstr);
# else /* XLAT_ABBREV */
	printf("ioprio_set(IOPRIO_WHO_PGRP, 0"
	       ", IOPRIO_PRIO_VALUE(IOPRIO_CLASS_NONE, 8191)) = %s\n",
	       errstr);
# endif

	rc = syscall(__NR_ioprio_set, bogus_which, bogus_who, bogus_ioprio);
	errstr = sprintrc(rc);
# if XLAT_RAW
	printf("ioprio_set(%#x, %d, %d) = %s\n",
	       (int) bogus_which, (int) bogus_who, (int) bogus_ioprio,
	       errstr);
# elif XLAT_VERBOSE
	printf("ioprio_set(%#x /* IOPRIO_WHO_??? */, %d, %d /* %s */) = %s\n",
	       (int) bogus_which, (int) bogus_who, (int) bogus_ioprio,
	       bogus_ioprio_str, errstr);
# else /* XLAT_ABBREV */
	printf("ioprio_set(%#x /* IOPRIO_WHO_??? */, %d, %s) = %s\n",
	       (int) bogus_which, (int) bogus_who, bogus_ioprio_str,
	       errstr);
# endif

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_ioprio_get && __NR_ioprio_set");

#endif
