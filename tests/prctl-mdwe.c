/*
 * Check decoding of prctl PR_GET_MDWE/PR_SET_MDWE operations.
 *
 * Copyright (c) 2021-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/prctl.h>

static long injected_val;

static long
do_prctl(kernel_ulong_t cmd, kernel_ulong_t arg2, kernel_ulong_t arg3,
	 kernel_ulong_t arg4, kernel_ulong_t arg5)
{
	long rc = syscall(__NR_prctl, cmd, arg2, arg3, arg4, arg5);

	if (rc != injected_val) {
		error_msg_and_fail("Return value (%ld) differs from expected "
						   "injected value (%ld)",
						   rc, injected_val);
	}

	return rc;
}

int
main(int argc, char **argv)
{
	static const kernel_ulong_t bogus_arg2 =
			(kernel_ulong_t) 0xdecafeedbeefda7eULL;
	static const kernel_ulong_t bogus_arg3 =
			(kernel_ulong_t) 0xbadc0dedfacef00dULL;
	static const kernel_ulong_t bogus_arg4 =
			(kernel_ulong_t) 0xcafeda7efeedbeefULL;
	static const kernel_ulong_t bogus_arg5 =
			(kernel_ulong_t) 0xfa57beeffacefeedULL;

	static const struct {
		long arg;
		const char *str;
	} get_strs[] = {
			{-1,         ""},
			{0,          ""},
			{1,          " (PR_MDWE_REFUSE_EXEC_GAIN)"},
			{2,          " (PR_MDWE_NO_INHERIT)"},
			{3,          " (PR_MDWE_REFUSE_EXEC_GAIN|PR_MDWE_NO_INHERIT)"},
			{4,          ""},
			{5,          " (PR_MDWE_REFUSE_EXEC_GAIN|0x4)"},
			{0x7ea1cafc, ""},
	};
	static const struct {
		kernel_ulong_t arg;
		const char *str;
	} set_strs[] = {
			{0,          "0"},
			{1,          "PR_MDWE_REFUSE_EXEC_GAIN"},
			{2,          "PR_MDWE_NO_INHERIT"},
			{3,          "PR_MDWE_REFUSE_EXEC_GAIN|PR_MDWE_NO_INHERIT"},
			{4,          "0x4 /* PR_MDWE_??? */"},
			{5,          "PR_MDWE_REFUSE_EXEC_GAIN|0x4"},
			{0x7ea1cafc, "0x7ea1cafc /* PR_MDWE_??? */"}
	};

	long rc;
	unsigned long num_skip;
	const char *str = NULL;
	bool locked = false;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);
	injected_val = strtol(argv[2], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		if ((prctl_marker() != injected_val) ||
		    ((injected_val == -1) && (errno != ENOTTY)))
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Have not locked on prctl(-1, -2, -3, -4"
				   ", -5) returning %ld", injected_val);

	/* PR_GET_MDWE */
	for (unsigned int j = 0; j < 2; j++) {
		if (j) {
			rc = do_prctl(PR_GET_MDWE, bogus_arg2, bogus_arg3,
						   bogus_arg4, bogus_arg5);
		} else {
			rc = do_prctl(PR_GET_MDWE, 0, 0, 0, 0);
		}

		const char *errstr = sprintrc(rc);

		for (size_t i = 0; i < ARRAY_SIZE(get_strs); i++) {
			if (get_strs[i].arg == rc) {
				str = get_strs[i].str;
				break;
			}
		}
		if (!str)
			error_msg_and_fail("Unknown return value: %ld", rc);

		printf("prctl(PR_GET_MDWE, %s) = ", j ?
#if SIZEOF_KERNEL_LONG_T > 4
		       "0xdecafeedbeefda7e, 0xbadc0dedfacef00d"
		       ", 0xcafeda7efeedbeef, 0xfa57beeffacefeed"
#else
		       "0xbeefda7e, 0xfacef00d, 0xfeedbeef, 0xfacefeed"
#endif
		       : "0, 0, 0, 0");
		if (rc < 0)
			printf("%s", errstr);
		else
			printf("%#lx", rc);
		printf("%s (INJECTED)\n", str);
	}

	/* PR_SET_FP_MODE */
	for (size_t i = 0; i < ARRAY_SIZE(set_strs); i++) {
		rc = do_prctl(PR_SET_MDWE, set_strs[i].arg, 0, 0, 0);
		printf("prctl(PR_SET_MDWE, %s, 0, 0, 0) = %s (INJECTED)\n",
		       set_strs[i].str, sprintrc(rc));

		rc = do_prctl(PR_SET_MDWE, set_strs[i].arg,
			      bogus_arg3, bogus_arg4, bogus_arg5);
		printf("prctl(PR_SET_MDWE, %s, %s) = %s (INJECTED)\n",
		       set_strs[i].str,
#if SIZEOF_KERNEL_LONG_T > 4
		       "0xbadc0dedfacef00d, 0xcafeda7efeedbeef"
		       ", 0xfa57beeffacefeed"
#else
		       "0xfacef00d, 0xfeedbeef, 0xfacefeed"
#endif
		       , sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
