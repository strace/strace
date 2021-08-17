/*
 * Check decoding of prctl PR_GET_FP_MODE/PR_SET_FP_MODE operations.
 *
 * Copyright (c) 2021 The strace developers.
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
do_prctl(kernel_ulong_t cmd, kernel_ulong_t arg2, kernel_ulong_t arg3)
{
	long rc = syscall(__NR_prctl, cmd, arg2, arg3);

	if (rc != injected_val)
		error_msg_and_fail("Return value (%ld) differs from expected "
						   "injected value (%ld)",
						   rc, injected_val);

	return rc;
}

int
main(int argc, char **argv)
{
	static const kernel_ulong_t bogus_arg2 =
			(kernel_ulong_t) 0xdecafeedbeefda7eULL;
	static const kernel_ulong_t bogus_arg3 =
			(kernel_ulong_t) 0xdecafeedbeefda7eULL;

	static const struct {
		long arg;
		const char *str;
	} get_strs[] = {
			{-1,       ""},
			{0,        ""},
			{1,        " (PR_FP_MODE_FR)"},
			{2,        " (PR_FP_MODE_FRE)"},
			{3,        " (PR_FP_MODE_FR|PR_FP_MODE_FRE)"},
			{0x20,     ""},
			{0x20 | 3, " (PR_FP_MODE_FR|PR_FP_MODE_FRE|0x20)"}
	};
	static const struct {
		kernel_ulong_t arg;
		const char *str;
	} set_strs[] = {
			{0,        "0"},
			{1,        "PR_FP_MODE_FR"},
			{2,        "PR_FP_MODE_FRE"},
			{3,        "PR_FP_MODE_FR|PR_FP_MODE_FRE"},
			{0x20,     "0x20 /* PR_FP_MODE_??? */"},
			{0x20 | 3, "PR_FP_MODE_FR|PR_FP_MODE_FRE|0x20"}
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

	/* PR_GET_FP_MODE */
	rc = do_prctl(PR_GET_FP_MODE, bogus_arg2, bogus_arg3);

	for (size_t i = 0; i < ARRAY_SIZE(get_strs); i++) {
		if (get_strs[i].arg == rc) {
			str = get_strs[i].str;
			break;
		}
	}
	if (!str)
		error_msg_and_fail("Unknown return value: %ld", rc);

	if (rc < 0) {
		printf("prctl(PR_GET_FP_MODE) = %s%s (INJECTED)\n",
		       sprintrc(rc), str);
	} else {
		printf("prctl(PR_GET_FP_MODE) = %#lx%s (INJECTED)\n",
		       rc, str);
	}

	/* PR_SET_FP_MODE */
	for (size_t i = 0; i < ARRAY_SIZE(set_strs); i++) {
		rc = do_prctl(PR_SET_FP_MODE, set_strs[i].arg, bogus_arg3);

		printf("prctl(PR_SET_FP_MODE, %s) = %s (INJECTED)\n", set_strs[i].str,
			   sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
