/*
 * Check decoding of s390_runtime_instr syscall.
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
		{ 0, "SIG_0" },
		{ (kernel_ulong_t) 0xfacefeedac0ffeedULL, NULL },
		{ ARG_STR(SIGALRM) },
		{ 33, "SIGRT_1" },
		{ 63, "SIGRT_31" },
	};

	unsigned int i;
	long rc;

	for (i = 0; i < ARRAY_SIZE(cmd_args); i++) {
		rc = syscall(__NR_s390_runtime_instr, cmd_args[i].cmd, 0xdead);
		printf("s390_runtime_instr(%s) = %s\n",
		       cmd_args[i].cmd_str, sprintrc(rc));
	}

	for (i = 0; i < ARRAY_SIZE(start_sig_args); i++) {
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
