/*
 * Check decoding of prctl PR_GET_TID_ADDRESS operation.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <linux/prctl.h>

#if defined __NR_prctl && defined __NR_set_tid_address && \
	defined PR_GET_TID_ADDRESS

# include <inttypes.h>
# include <stdio.h>
# include <unistd.h>

static const char *
sprintaddr(kernel_ulong_t addr)
{
	static char buf[sizeof("0x") + sizeof(addr) * 2];

	if (addr) {
		snprintf(buf, sizeof(buf), "%#llx", (unsigned long long) addr);

		return buf;
	}

	return "NULL";
}

int
main(void)
{
	static const kernel_ulong_t bogus_addr =
		(kernel_ulong_t) 0xfffffffffffffffdULL;

	/* Note that kernel puts kernel-sized pointer even on x32 */
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_ulong_t, ptr);
	long rc;
	long set_ok;

	*ptr = (kernel_ulong_t) 0xbadc0dedda7a1057ULL;

	rc = syscall(__NR_prctl, PR_GET_TID_ADDRESS, NULL);
	printf("prctl(PR_GET_TID_ADDRESS, NULL) = %s\n", sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_TID_ADDRESS, bogus_addr);
	printf("prctl(PR_GET_TID_ADDRESS, %#llx) = %s\n",
	       (unsigned long long) bogus_addr, sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_TID_ADDRESS, ptr);
	if (rc) {
		printf("prctl(PR_GET_TID_ADDRESS, %p) = %s\n",
		       ptr, sprintrc(rc));
	} else {
		printf("prctl(PR_GET_TID_ADDRESS, [%s]) = %s\n",
		       sprintaddr(*ptr), sprintrc(rc));
	}

	set_ok = syscall(__NR_set_tid_address, bogus_addr);

	rc = syscall(__NR_prctl, PR_GET_TID_ADDRESS, ptr);
	if (rc) {
		printf("prctl(PR_GET_TID_ADDRESS, %p) = %s\n",
		       ptr, sprintrc(rc));
	} else {
		printf("prctl(PR_GET_TID_ADDRESS, [%s]) = %s\n",
		       sprintaddr(set_ok ? bogus_addr : *ptr), sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl && __NR_set_tid_address && PR_GET_TID_ADDRESS")

#endif
