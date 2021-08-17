/*
 * Check decoding of prctl PR_SET_TAGGED_ADDR_CTRL and PR_GET_TAGGED_ADDR_CTRL
 * operations.
 *
 * Copyright (c) 2021 The strace developers.
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

#if !XLAT_RAW
static void
print_tagged_addr_arg(kernel_ulong_t arg)
{
	kernel_ulong_t val = arg & PR_TAGGED_ADDR_ENABLE;

	printf("%sPR_TAGGED_ADDR_ENABLE", val ? "" : "!");
	arg &= ~val;
	val = arg & PR_MTE_TCF_MASK;
	switch (val) {
	case PR_MTE_TCF_NONE:  printf("|PR_MTE_TCF_NONE");  break;
	case PR_MTE_TCF_SYNC:  printf("|PR_MTE_TCF_SYNC");  break;
	case PR_MTE_TCF_ASYNC: printf("|PR_MTE_TCF_ASYNC"); break;
	case PR_MTE_TCF_MASK:  printf("|PR_MTE_TCF_MASK");  break;
	}
	arg &= ~val;
	val = arg & PR_MTE_TAG_MASK;
	if (val) {
		printf("|%#llx<<PR_MTE_TAG_SHIFT",
		       (unsigned long long) val >> PR_MTE_TAG_SHIFT);
	}
	arg &= ~val;
	if (arg)
		printf("|%#llx", (unsigned long long) arg);
}
#endif /* !XLAT_RAW */

#ifdef INJECT_RETVAL
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif

#if SIZEOF_KERNEL_LONG_T == 8
# define HIBITS(a) a
#else
# define HIBITS(a)
#endif

int
main(int argc, char *argv[])
{
	prctl_marker();

#ifdef INJECT_RETVAL
	unsigned long num_skip;
	long inject_retval;
	bool locked = false;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);
	inject_retval = strtol(argv[2], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		if (prctl_marker() != inject_retval)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Have not locked on prctl(-1, -2, -3, -4"
				   ", -5) returning %ld", inject_retval);
#endif /* INJECT_RETVAL */

	static const struct {
		kernel_ulong_t val;
		const char *str;
	} args[] = {
		{ 0, "!PR_TAGGED_ADDR_ENABLE|PR_MTE_TCF_NONE" },
		{ /* 1 */ ARG_STR(PR_TAGGED_ADDR_ENABLE|PR_MTE_TCF_NONE) },
		{ 2, "!PR_TAGGED_ADDR_ENABLE|PR_MTE_TCF_SYNC" },
		{ /* 5 */ ARG_STR(PR_TAGGED_ADDR_ENABLE|PR_MTE_TCF_ASYNC) },
		{ 6, "!PR_TAGGED_ADDR_ENABLE|PR_MTE_TCF_MASK" },
		{ 8, "!PR_TAGGED_ADDR_ENABLE|PR_MTE_TCF_NONE|0x1<<PR_MTE_TAG_SHIFT" },
		{ /* 13 */ ARG_STR(PR_TAGGED_ADDR_ENABLE|PR_MTE_TCF_ASYNC|0x1<<PR_MTE_TAG_SHIFT) },
		{ 0x57ae57f4, "!PR_TAGGED_ADDR_ENABLE|PR_MTE_TCF_ASYNC"
			      "|0xcafe<<PR_MTE_TAG_SHIFT|0x57a80000" },
		{ (kernel_ulong_t) -1LLU, "PR_TAGGED_ADDR_ENABLE"
					  "|PR_MTE_TCF_MASK"
					  "|0xffff<<PR_MTE_TAG_SHIFT"
					  "|0x" HIBITS("ffffffff") "fff80000" },
	};
	long rc;
	size_t i = 0;

	for (i = 0; i < ARRAY_SIZE(args); i++) {
		rc = syscall(__NR_prctl, PR_SET_TAGGED_ADDR_CTRL,
			     args[i].val, 1, 2, 3);
		printf("prctl(" XLAT_KNOWN(0x37, "PR_SET_TAGGED_ADDR_CTRL") ", "
		       XLAT_FMT_LL ", 0x1, 0x2, 0x3) = %s" INJ_STR "\n",
		       XLAT_SEL((unsigned long long) args[i].val, args[i].str),
		       sprintrc(rc));
	}

	rc = syscall(__NR_prctl, PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
	const char *errstr = sprintrc(rc);
	printf("prctl(" XLAT_KNOWN(0x38, "PR_GET_TAGGED_ADDR_CTRL")
	       ", 0, 0, 0, 0) = ");
	if (rc >= 0) {
		printf("%#lx", rc);
#if !XLAT_RAW
		for (i = 0; i < ARRAY_SIZE(args); i++) {
			if (args[i].val == (unsigned long) rc) {
				printf(" (%s)", args[i].str);
				break;
			}
		}

		if (i == ARRAY_SIZE(args)) {
			printf(" (");
			print_tagged_addr_arg(rc);
			printf(")");
		}
#endif /* !XLAT_RAW */
		puts(INJ_STR);
	} else {
		printf("%s" INJ_STR "\n", errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
