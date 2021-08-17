/*
 * Check decoding of prctl PR_PAC_RESET_KEYS operation.
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

#include "xlat.h"
#include "xlat/pr_pac_enabled_keys.h"

#ifdef INJECT_RETVAL
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif

#define PR_PAC_ENABLE_FLAGS_MASK \
	(PR_PAC_APIAKEY|PR_PAC_APIBKEY|PR_PAC_APDAKEY|PR_PAC_APDBKEY)

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
		{ ARG_STR(0) },
		{ ARG_STR(PR_PAC_APIAKEY) },
		{ ARG_STR(PR_PAC_APIBKEY|PR_PAC_APDAKEY|PR_PAC_APDBKEY|0x10) },
		{ 0x7ffffff0, NVERB("0x7ffffff0 /* ") "PR_PAC_???" NVERB(" */") },
	};
	long rc;
	const char *errstr;
	size_t i = 0;

	for (i = 0; i < ARRAY_SIZE(args); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(args); j++) {
			rc = syscall(__NR_prctl, PR_PAC_SET_ENABLED_KEYS,
				     args[i].val, args[j].val, 0, 1);
			errstr = sprintrc(rc);
			printf("prctl("
			       XLAT_KNOWN(0x3c, "PR_PAC_SET_ENABLED_KEYS"));
			if (args[i].val) {
				printf(", " XLAT_FMT_LL,
				       XLAT_SEL((unsigned long long) args[i].val,
						args[i].str));
			} else {
				printf(", 0");
			}
			if (args[j].val) {
				printf(", " XLAT_FMT_LL,
				       XLAT_SEL((unsigned long long) args[j].val,
						args[j].str));
			} else {
				printf(", 0");
			}
			printf(", 0, 0x1) = %s" INJ_STR "\n", errstr);
		}
	}

	rc = syscall(__NR_prctl, PR_PAC_GET_ENABLED_KEYS, 0, 0, 0, 0);
	errstr = sprintrc(rc);
	printf("prctl(" XLAT_KNOWN(0x3d, "PR_PAC_GET_ENABLED_KEYS")
	       ", 0, 0, 0, 0) = ");

	if (rc > 0) {
		printf("%#lx", rc);
#if !XLAT_RAW
		for (i = 0; i < ARRAY_SIZE(args); i++) {
			if (!(rc & PR_PAC_ENABLE_FLAGS_MASK))
				break;

			if (args[i].val == (unsigned long) rc) {
				printf(" (%s)", args[i].str);
				break;
			}
		}

		if (i == ARRAY_SIZE(args)) {
			printf(" (");
			printflags(pr_pac_enabled_keys, rc, "PR_PAC_???");
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
