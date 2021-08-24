/*
 * Check decoding of prlimit64 syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

#include "pidns.h"
#include "xlat.h"
#include "xlat/resources.h"

#ifndef RETVAL_INJECTED
# define RETVAL_INJECTED 0
#endif

#if RETVAL_INJECTED
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif
#define INJECT_RETVAL 42

static const char *
sprint_rlim(uint64_t lim)
{
	if (lim == -1ULL)
		return "RLIM64_INFINITY";

	static char buf[2][sizeof(lim)*3 + sizeof("*1024")];
	static int i;
	i &= 1;
	if (lim > 1024 && lim % 1024 == 0)
		sprintf(buf[i], "%" PRIu64 "*1024", lim / 1024);
	else
		sprintf(buf[i], "%" PRIu64, lim);

	return buf[i++];
}

int
main(int argc, char *argv[])
{
	PIDNS_TEST_INIT;

	unsigned long pid =
		(unsigned long) 0xdefaced00000000ULL | (unsigned) getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);
	uint64_t *const rlimit = tail_alloc(sizeof(*rlimit) * 2);
	const struct xlat_data *xlat;
	long rc;
	unsigned long num_skip = 256;
	size_t i = 0;

	if (argc >= 2)
		num_skip = strtoul(argv[1], NULL, 0);

	for (i = 0; i < num_skip; i++) {
		long rc = syscall(__NR_prlimit64, 0, 16, 0, 0);
		pidns_print_leader();
		printf("prlimit64(0, 0x10 /* RLIMIT_??? */, NULL, NULL)"
		       " = %s%s\n",
		       sprintrc(rc), rc == INJECT_RETVAL ? INJ_STR : "");
	}

	/* The shortest output */
	rc = syscall(__NR_prlimit64, 0, RLIMIT_AS, 0, 0);
	pidns_print_leader();
	printf("prlimit64(0, RLIMIT_AS, NULL, NULL) = %s" INJ_STR "\n",
	       sprintrc(rc));

	for (i = 0, xlat = resources->data; i < resources->size; ++xlat, ++i) {
		if (!xlat->str)
			continue;

		unsigned long res = 0xfacefeed00000000ULL | xlat->val;
		rc = syscall(__NR_prlimit64, pid, res, 0, rlimit);
		pidns_print_leader();
		if (!RETVAL_INJECTED && (rc < 0)) {
			printf("prlimit64(%d%s, %s, NULL, %p) = %s\n",
			       (unsigned) pid, pid_str,
			       xlat->str, rlimit,
			       sprintrc(rc));
		} else {
			printf("prlimit64(%d%s, %s, NULL"
			       ", {rlim_cur=%s, rlim_max=%s})"
			       " = %d" INJ_STR "\n",
			       (unsigned) pid, pid_str,
			       xlat->str,
			       sprint_rlim(rlimit[0]),
			       sprint_rlim(rlimit[1]),
			       RETVAL_INJECTED ? INJECT_RETVAL : 0);
		}
	}

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
