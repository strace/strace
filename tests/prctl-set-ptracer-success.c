/*
 * Check decoding of prctl PR_SET_PTRACER operation.
 *
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/prctl.h>

#include "pidns.h"


int
main(int argc, char *argv[])
{
	PIDNS_TEST_INIT;

	long rc;
	unsigned long num_skip = 256;
	bool locked = false;

	if (argc >= 2)
		num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		rc = prctl_marker();
#ifdef PIDNS_TRANSLATION
		const char *errstr = sprintrc(rc);
		pidns_print_leader();
		printf("prctl(" XLAT_UNKNOWN(0xffffffff, "PR_???")
		       ", 0xfffffffe, 0xfffffffd, 0xfffffffc, 0xfffffffb) = ");

		if (rc < 0) {
			puts(errstr);
		} else {
			printf("%ld (INJECTED)\n", rc);
		}
#endif

		if (rc < 0)
			continue;

		locked = true;
		break;
	}

	rc = syscall(__NR_prctl, PR_SET_PTRACER, F8ILL_KULONG_MASK,
		     0xdead, 0xface, 0xbeef);
	pidns_print_leader();
	printf("prctl(" XLAT_KNOWN(0x59616d61, "PR_SET_PTRACER")
	       ", 0) = %s%s\n", sprintrc(rc), locked ? " (INJECTED)" : "");

	rc = syscall(__NR_prctl, PR_SET_PTRACER,
		     (kernel_ulong_t) 0xbadc0dedffffffffULL,
		     0xdead, 0xface, 0xbeef);
	pidns_print_leader();
	printf("prctl(" XLAT_KNOWN(0x59616d61, "PR_SET_PTRACER") ", "
	       XLAT_KNOWN(-1, "PR_SET_PTRACER_ANY") ") = %s%s\n",
	       sprintrc(rc), locked ? " (INJECTED)" : "");

	const int pid = getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);
	rc = syscall(__NR_prctl, PR_SET_PTRACER, F8ILL_KULONG_MASK | pid,
		     0xdead, 0xface, 0xbeef);
	pidns_print_leader();
	printf("prctl(" XLAT_KNOWN(0x59616d61, "PR_SET_PTRACER") ", %d%s)"
	       " = %s%s\n",
	       pid, pid_str, sprintrc(rc), locked ? " (INJECTED)" : "");

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
