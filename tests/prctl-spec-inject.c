/*
 * Check decoding of PR_SET_SPECULATION_CTRL and PR_GET_SPECULATION_CTRL
 * prctl operations.
 *
 * Copyright (c) 2018-2021 The strace developers.
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

#if SIZEOF_KERNEL_LONG_T > 4
# define BOGUS_ARG4 ((kernel_ulong_t) 0xdeadfacebadbeefdULL)
# define BOGUS_ARG5 ((kernel_ulong_t) 0xcafedeadfacefeedULL)
# define BOGUS_ARGS_STR ", 0xdeadfacebadbeefd, 0xcafedeadfacefeed"
#else
# define BOGUS_ARG4 0xfacedbad
# define BOGUS_ARG5 0xdeadcafe
# define BOGUS_ARGS_STR ", 0xfacedbad, 0xdeadcafe"
#endif

static long
do_prctl(kernel_ulong_t cmd, kernel_ulong_t arg2, kernel_ulong_t arg3)
{
	long rc = syscall(__NR_prctl, cmd, arg2, arg3, BOGUS_ARG4, BOGUS_ARG5);

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
		(kernel_ulong_t) 0xdeadfacebadc0dedULL;
	static const kernel_ulong_t bogus_arg3 =
		(kernel_ulong_t) 0xdecafeedbeefda7eULL;

	static const struct {
		long arg;
		const char *str;
	} spec_strs[] = {
		{ 0, "PR_SPEC_STORE_BYPASS" },
		{ 1, "PR_SPEC_INDIRECT_BRANCH" },
		{ 2, "PR_SPEC_L1D_FLUSH" },
	};

	static const struct {
		long arg;
		const char *str;
	} get_strs[] = {
		{ -1, "" },
		{ 0, " (PR_SPEC_NOT_AFFECTED)" },
		{ 1, " (PR_SPEC_PRCTL)" },
		{ 3, " (PR_SPEC_PRCTL|PR_SPEC_ENABLE)" },
		{ 8, " (PR_SPEC_FORCE_DISABLE)" },
		{ 16, " (PR_SPEC_DISABLE_NOEXEC)" },
		{ 32, "" },
		{ 42, " (PR_SPEC_ENABLE|PR_SPEC_FORCE_DISABLE|0x20)" },
	};
	static const struct {
		kernel_ulong_t arg;
		const char *str;
	} set_strs[] = {
		{ 0, "0 /* PR_SPEC_??? */" },
		{ 1, "0x1 /* PR_SPEC_??? */" },
		{ 2, "PR_SPEC_ENABLE" },
		{ 3, "0x3 /* PR_SPEC_??? */" },
		{ 8, "PR_SPEC_FORCE_DISABLE" },
		{ 16, "PR_SPEC_DISABLE_NOEXEC" },
		{ 32, "0x20 /* PR_SPEC_??? */" },
		{ (kernel_ulong_t) 0xdecafeedbeefda7eULL, "0x"
#if SIZEOF_KERNEL_LONG_T == 8
			"decafeed"
#endif
			"beefda7e /* PR_SPEC_??? */" },
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

	/* PR_GET_SPECULATION_CTRL */
	rc = do_prctl(52, 3, bogus_arg3);
	printf("prctl(PR_GET_SPECULATION_CTRL, 0x3 /* PR_SPEC_??? */, %#llx"
	       BOGUS_ARGS_STR ") = %s (INJECTED)\n",
	       (unsigned long long) bogus_arg3, sprintrc(rc));

	rc = do_prctl(52, bogus_arg2, bogus_arg3);
	printf("prctl(PR_GET_SPECULATION_CTRL, %#llx /* PR_SPEC_??? */, %#llx"
	       BOGUS_ARGS_STR ") = %s (INJECTED)\n",
	       (unsigned long long) bogus_arg2, (unsigned long long) bogus_arg3,
	       sprintrc(rc));

	for (unsigned c = 0; c < ARRAY_SIZE(spec_strs); c++) {
		rc = do_prctl(52, spec_strs[c].arg, bogus_arg3);

		for (unsigned i = 0; i < ARRAY_SIZE(get_strs); i++) {
			if (get_strs[i].arg == rc) {
				str = get_strs[i].str;
				break;
			}
		}
		if (!str)
			error_msg_and_fail("Unknown return value: %ld", rc);

		if (rc < 0) {
			printf("prctl(PR_GET_SPECULATION_CTRL, %s, %#llx"
			       BOGUS_ARGS_STR ") = %s%s (INJECTED)\n",
			       spec_strs[c].str,
			       (unsigned long long) bogus_arg3,
			       sprintrc(rc), str);
		} else {
			printf("prctl(PR_GET_SPECULATION_CTRL, %s, %#llx"
			       BOGUS_ARGS_STR ") = %#lx%s (INJECTED)\n",
			       spec_strs[c].str,
			       (unsigned long long) bogus_arg3, rc, str);
		}
	}


	/* PR_SET_SPECULATION_CTRL*/
	rc = do_prctl(53, 3, bogus_arg3);
	printf("prctl(PR_SET_SPECULATION_CTRL, 0x3 /* PR_SPEC_??? */, %#llx"
	       BOGUS_ARGS_STR ") = %s (INJECTED)\n",
	       (unsigned long long) bogus_arg3, sprintrc(rc));

	rc = do_prctl(53, bogus_arg2, bogus_arg3);
	printf("prctl(PR_SET_SPECULATION_CTRL, %#llx /* PR_SPEC_??? */, %#llx"
	       BOGUS_ARGS_STR ") = %s (INJECTED)\n",
	       (unsigned long long) bogus_arg2,
	       (unsigned long long) bogus_arg3,
	       sprintrc(rc));

	for (unsigned c = 0; c < ARRAY_SIZE(spec_strs); c++) {
		for (unsigned i = 0; i < ARRAY_SIZE(set_strs); i++) {
			rc = do_prctl(53, spec_strs[c].arg, set_strs[i].arg);
			printf("prctl(PR_SET_SPECULATION_CTRL, %s, %s"
			       BOGUS_ARGS_STR ") = %s (INJECTED)\n",
			       spec_strs[c].str, set_strs[i].str, sprintrc(rc));
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
