/*
 * Check decoding of SPARC-specific kern_features syscall.
 *
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"


#include "raw_syscall.h"
#include "scno.h"

#if defined __NR_kern_features && defined raw_syscall_0

# include <errno.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

static void
test_kern_features(unsigned int num)
{
	/* The expected return codes, as enforced by fault injection */
	static struct {
		kernel_ulong_t ret;
		const char *str;
	} checks[] = {
		{ 0, NULL },
		{ 1, "KERN_FEATURE_MIXED_MODE_STACK" },
		{ 2, "0x2" },
		{ 0x7ffffffe, "0x7ffffffe" },
		{ 0x7fffffff, "KERN_FEATURE_MIXED_MODE_STACK|0x7ffffffe" },
		{ (kernel_ulong_t) 0xbadc0deddeadfaceULL,
			sizeof(kernel_ulong_t) == 8 ?
			"0xbadc0deddeadface" : "0xdeadface" },
		{ (kernel_ulong_t) -1ULL,
			sizeof(kernel_ulong_t) == 8 ?
			"KERN_FEATURE_MIXED_MODE_STACK|0xfffffffffffffffe" :
			"KERN_FEATURE_MIXED_MODE_STACK|0xfffffffe" },
	};

	kernel_ulong_t err = 0;
	kernel_ulong_t rc = raw_syscall_0(__NR_kern_features, &err);

	if (err) {
		errno = rc;
		printf("kern_features() = %s\n", sprintrc(-1));
		return;
	}

	printf("kern_features() = %#llx", (unsigned long long) rc);

	if (num < ARRAY_SIZE(checks)) {
		if (rc != checks[num].ret)
			error_msg_and_fail("Expected return value (%llx) "
					   "doesn't match expected one (%#llx)",
					   (unsigned long long) rc,
					   (unsigned long long) checks[num].ret);

		if (checks[num].str)
			printf(" (%s)", checks[num].str);

		printf(" (INJECTED)");
	} else if (rc) {
		printf(" (");

		if (rc & 1)
			printf("KERN_FEATURE_MIXED_MODE_STACK");

		if (rc & ~1ULL) {
			if (rc & 1)
				printf("|");

			printf("%#llx", rc & ~1ULL);
		}

		printf(")");
	}

	puts("");
}

int
main(int ac, char **av)
{
	test_kern_features(ac > 1 ? atoi(av[1]) : -1);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_kern_features && raw_syscall_0");

#endif
