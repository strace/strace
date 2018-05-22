/*
 * Check decoding of PR_SET_SPECULATION_CTRL and PR_GET_SPECULATION_CTRL
 * prctl operations.
 *
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_prctl

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <linux/prctl.h>

static long injected_val;

long
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
		(kernel_ulong_t) 0xdeadfacebadc0dedULL;
	static const kernel_ulong_t bogus_arg3 =
		(kernel_ulong_t) 0xdecafeedbeefda7eULL;
	static const struct {
		long arg;
		const char *str;
	} get_strs[] = {
		{ -1, "" },
		{ 0, " (PR_SPEC_NOT_AFFECTED)" },
		{ 1, " (PR_SPEC_PRCTL)" },
		{ 3, " (PR_SPEC_PRCTL|PR_SPEC_ENABLE)" },
		{ 8, " (PR_SPEC_FORCE_DISABLE)" },
		{ 16, " (0x10)" },
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
		{ 16, "0x10 /* PR_SPEC_??? */" },
		{ (kernel_ulong_t) 0xdecafeedbeefda7eULL, "0x"
# if SIZEOF_KERNEL_LONG_T == 8
			"decafeed"
# endif
			"beefda7e /* PR_SPEC_??? */" },
	};

	long rc;
	const char *str = NULL;

	if (argc < 2)
		error_msg_and_fail("Usage: %s INJECTED_VAL", argv[0]);

	injected_val = strtol(argv[1], NULL, 0);

	/* PR_GET_SPECULATION_CTRL */
	rc = do_prctl(52, 1, bogus_arg3);
	printf("prctl(PR_GET_SPECULATION_CTRL, 0x1 /* PR_SPEC_??? */) "
	       "= %s (INJECTED)\n", sprintrc(rc));

	rc = do_prctl(52, bogus_arg2, bogus_arg3);
	printf("prctl(PR_GET_SPECULATION_CTRL, %#llx /* PR_SPEC_??? */) "
	       "= %s (INJECTED)\n",
	       (unsigned long long) bogus_arg2, sprintrc(rc));

	rc = do_prctl(52, 0, bogus_arg3);

	for (unsigned i = 0; i < ARRAY_SIZE(get_strs); i++) {
		if (get_strs[i].arg == rc) {
			str = get_strs[i].str;
			break;
		}
	}
	if (!str)
		error_msg_and_fail("Unknown return value: %ld", rc);

	printf("prctl(PR_GET_SPECULATION_CTRL, PR_SPEC_STORE_BYPASS) "
	       "= %s%s (INJECTED)\n", sprintrc(rc), str);


	/* PR_SET_SPECULATION_CTRL*/
	rc = do_prctl(53, 1, bogus_arg3);
	printf("prctl(PR_SET_SPECULATION_CTRL, 0x1 /* PR_SPEC_??? */, %#llx) "
	       "= %s (INJECTED)\n",
	       (unsigned long long) bogus_arg3, sprintrc(rc));

	rc = do_prctl(53, bogus_arg2, bogus_arg3);
	printf("prctl(PR_SET_SPECULATION_CTRL, %#llx /* PR_SPEC_??? */, %#llx) "
	       "= %s (INJECTED)\n",
	       (unsigned long long) bogus_arg2,
	       (unsigned long long) bogus_arg3,
	       sprintrc(rc));

	for (unsigned i = 0; i < ARRAY_SIZE(set_strs); i++) {
		rc = do_prctl(53, 0, set_strs[i].arg);
		printf("prctl(PR_SET_SPECULATION_CTRL, PR_SPEC_STORE_BYPASS"
		       ", %s) = %s (INJECTED)\n",
		       set_strs[i].str, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl")

#endif
