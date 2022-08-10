/*
 * Check decoding of prctl PR_SME_SET_VL/PR_SME_GET_VL operations.
 *
 * Copyright (c) 2021-2022 The strace developers.
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

#ifdef INJECT_RETVAL
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif

#ifndef EXT
# define EXT SME
#endif

#define EXT_STR STRINGIFY_VAL(EXT)
#define GLUE_(a_, b_, c_) a_ ## b_ ## c_
#define GLUE(a_, b_, c_) GLUE_(a_, b_, c_)
#define _(pfx_, sfx_) GLUE(pfx_, EXT, sfx_)

#if !XLAT_RAW
static void
print_sme_vl_arg(kernel_ulong_t arg)
{
	kernel_ulong_t flags = arg & ~_(PR_, _VL_LEN_MASK);

	if (arg < 0x10000)
		return;

	printf(" (");

	if (flags & _(PR_, _SET_VL_ONEXEC))
		printf("PR_" EXT_STR "_SET_VL_ONEXEC");
	if (flags & _(PR_, _VL_INHERIT)) {
		printf("%sPR_" EXT_STR "_VL_INHERIT",
		       flags & _(PR_, _SET_VL_ONEXEC) ? "|" : "");
	}

	kernel_ulong_t leftover =
			flags & ~(_(PR_, _SET_VL_ONEXEC)|_(PR_, _VL_INHERIT));
	if (leftover) {
		printf("%s%#llx",
		       leftover == flags ? "" : "|",
		       (unsigned long long) leftover);
	}

	kernel_ulong_t lens = arg & _(PR_, _VL_LEN_MASK);
	printf("%s%#llx", flags ? "|" : "", (unsigned long long) lens);

	printf(")");
}

#endif /* !XLAT_RAW */

int
main(int argc, char *argv[])
{
	const char *errstr;
	long rc;
	size_t i;

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
		bool known;
	} args[] = {
		{ ARG_STR(0) },
		{ ARG_STR(0xdead) },
		{ ARG_XLAT_KNOWN(0x10000, "0x10000|0") },
		{ ARG_XLAT_KNOWN(0x2ea57, "PR_" EXT_STR "_VL_INHERIT|0xea57") },
		{ ARG_XLAT_KNOWN(0x40000, "PR_" EXT_STR "_SET_VL_ONEXEC|0") },
		{ ARG_XLAT_KNOWN(0xfacefeed, "PR_" EXT_STR "_SET_VL_ONEXEC"
					     "|PR_" EXT_STR "_VL_INHERIT"
					     "|0xfac80000|0xfeed") },
		{ ARG_XLAT_KNOWN(0xbad00000, "0xbad00000|0") },
		{ ARG_XLAT_KNOWN(0xde90ded, "0xde90000|0xded") },
		{ (kernel_ulong_t) 0xbadc0ded0000faceULL,
#if SIZEOF_KERNEL_LONG_T > 4
		  XLAT_KNOWN(0xbadc0ded0000face, "0xbadc0ded00000000|0xface")
#else
		  "0xface"
#endif
		},
	};

	for (i = 0; i < ARRAY_SIZE(args); i++) {
		rc = syscall(__NR_prctl, _(PR_, _SET_VL), args[i].val, 1, 2, 3);
		errstr = sprintrc(rc);
		printf("prctl(" XLAT_FMT ", %s) = ",
		       XLAT_SEL(_(PR_, _SET_VL), "PR_" EXT_STR "_SET_VL"),
		       args[i].str);
		if (rc >= 0) {
			printf("%#lx", rc);
#if !XLAT_RAW
			print_sme_vl_arg(rc);
#endif /* !XLAT_RAW */
			puts(INJ_STR);
		} else {
			printf("%s" INJ_STR "\n", errstr);
		}
	}

	rc = syscall(__NR_prctl, _(PR_, _GET_VL), 1, 2, 3, 4);
	errstr = sprintrc(rc);
	printf("prctl(" XLAT_FMT ") = ",
	       XLAT_SEL(_(PR_, _GET_VL), "PR_" EXT_STR "_GET_VL"));
	if (rc >= 0) {
		printf("%#lx", rc);
#if !XLAT_RAW
		print_sme_vl_arg(rc);
#endif /* !XLAT_RAW */
		puts(INJ_STR);
	} else {
		printf("%s" INJ_STR "\n", errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
