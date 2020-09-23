/*
 * Check decoding of ioprio_get and ioprio_set syscalls.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

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
	PIDNS_TEST_INIT;

	static const kernel_ulong_t bogus_which =
		(kernel_ulong_t) 0xdeadfacefa57beefULL;
	static const kernel_ulong_t bogus_who =
		(kernel_ulong_t) 0xbadc0dedda7a1057ULL;
	static const kernel_ulong_t bogus_ioprio =
		(kernel_ulong_t) 0xdec0ded1facefeedULL;

	const int pid = getpid();
	const int pgid = getpgid(0);

# if !XLAT_RAW
	static const char * const bogus_ioprio_str =
		"IOPRIO_PRIO_VALUE(0x7d677 /* IOPRIO_CLASS_??? */, 7917)";
# endif

	long rc;
	const char *errstr;

	rc = syscall(__NR_ioprio_get, bogus_which, bogus_who);
	errstr = sprintrc(rc);
	pidns_print_leader();
# if XLAT_RAW
	printf("ioprio_get(%#x, %d) = %s\n",
	       (int) bogus_which, (int) bogus_who, errstr);
# else /* XLAT_ABBREV || XLAT_VERBOSE */
	printf("ioprio_get(%#x /* IOPRIO_WHO_??? */, %d) = %s\n",
	       (int) bogus_which, (int) bogus_who, errstr);
# endif

	rc = syscall(__NR_ioprio_get, 1, pid);
	errstr = sprintrc(rc);
	pidns_print_leader();
	printf("ioprio_get(");
# if XLAT_RAW
	printf("0x1, ");
# elif XLAT_VERBOSE
	printf("0x1 /* IOPRIO_WHO_PROCESS */, ");
# else /* XLAT_ABBREV */
	printf("IOPRIO_WHO_PROCESS, ");
# endif
	printf("%d%s) = %s", pid, pidns_pid2str(PT_TGID), errstr);
# if !XLAT_RAW
	if (rc >= 0) {
		printf(" (IOPRIO_PRIO_VALUE(");
		printxval(ioprio_class, (unsigned int) rc >> 13,
			  "IOPRIO_CLASS_???");
		printf(", %u))", (unsigned int) rc & 0x1fff);
	}
# endif
	puts("");

	rc = syscall(__NR_ioprio_set, 2, pgid, 8191);
	errstr = sprintrc(rc);
	pidns_print_leader();
	printf("ioprio_set(");
# if XLAT_RAW
	printf("%#x", 2);
# elif XLAT_VERBOSE
	printf("%#x /* IOPRIO_WHO_PGRP */", 2);
# else /* XLAT_ABBREV */
	printf("IOPRIO_WHO_PGRP");
# endif
	printf(", %d%s", pgid, pidns_pid2str(PT_PGID));
# if XLAT_RAW
	printf(", 8191)");
# elif XLAT_VERBOSE
	printf(", 8191 /* IOPRIO_PRIO_VALUE(0 /* IOPRIO_CLASS_NONE */, 8191) */)");
# else /* XLAT_ABBREV */
	printf(", IOPRIO_PRIO_VALUE(IOPRIO_CLASS_NONE, 8191))");
# endif
	printf(" = %s\n", errstr);

	rc = syscall(__NR_ioprio_set, bogus_which, bogus_who, bogus_ioprio);
	errstr = sprintrc(rc);
	pidns_print_leader();
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

	pidns_print_leader();
	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_ioprio_get && __NR_ioprio_set");

#endif
