/*
 * Check decoding of s390_runtime_instr syscall.
 *
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_s390_runtime_instr

# include <errno.h>
# include <signal.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static struct {
		kernel_ulong_t cmd;
		const char * cmd_str;
	} cmd_args[] = {
		{ 0, "0 /* S390_RUNTIME_INSTR_??? */" },
		{ 4, "4 /* S390_RUNTIME_INSTR_??? */" },
		{ (kernel_ulong_t) 0xdeafbeefdeadc0deULL,
			"-559038242 /* S390_RUNTIME_INSTR_??? */" },
		{ 2, "S390_RUNTIME_INSTR_STOP"  },
	};

	static struct {
		kernel_ulong_t sig;
		const char * sig_str;
	} start_sig_args[] = {
		{ 0, "0" },
		{ (kernel_ulong_t) 0xfacefeedac0ffeedULL, NULL },
		{ ARG_STR(SIGALRM) },
		{ 33, "SIGRT_1" },
		{ 63, "SIGRT_31" },
	};

	long rc;

	for (unsigned int i = 0; i < ARRAY_SIZE(cmd_args); ++i) {
		rc = syscall(__NR_s390_runtime_instr, cmd_args[i].cmd, 0xdead);
		printf("s390_runtime_instr(%s) = %s\n",
		       cmd_args[i].cmd_str, sprintrc(rc));
	}

	for (unsigned int i = 0; i < ARRAY_SIZE(start_sig_args); ++i) {
		long saved_errno;

		rc = syscall(__NR_s390_runtime_instr, 1, start_sig_args[i].sig);
		saved_errno = errno;
		printf("s390_runtime_instr(S390_RUNTIME_INSTR_START, ");

		if (start_sig_args[i].sig_str)
			printf("%s", start_sig_args[i].sig_str);
		else
			printf("%d", (int) start_sig_args[i].sig);

		errno = saved_errno;
		printf(") = %s\n", sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_s390_runtime_instr")

#endif
