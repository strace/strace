/*
 * Check decoding of prctl operations without arguments and return code parsing:
 * PR_GET_KEEPCAPS, PR_GET_SECCOMP, PR_GET_TIMERSLACK, PR_GET_TIMING,
 * PR_TASK_PERF_EVENTS_DISABLE, and PR_TASK_PERF_EVENTS_ENABLE.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
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

#if defined __NR_prctl

# include <stdio.h>
# include <unistd.h>
# include <linux/prctl.h>

int
main(void)
{
	static const kernel_ulong_t bogus_op_bits =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	static const kernel_ulong_t bogus_arg =
		(kernel_ulong_t) 0xfacefeeddeadbeefULL;
	static const struct {
		kernel_ulong_t val;
		const char *str;
	} options[] = {
		{  7, "PR_GET_KEEPCAPS" },
		{ 13, "PR_GET_TIMING" },
		{ 21, "PR_GET_SECCOMP" },
		{ 30, "PR_GET_TIMERSLACK" },
		{ 31, "PR_TASK_PERF_EVENTS_DISABLE" },
		{ 32, "PR_TASK_PERF_EVENTS_ENABLE" },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, ptr);
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(options); i++) {
		long rc = syscall(__NR_prctl, options[i].val | bogus_op_bits,
				  bogus_arg);
		printf("prctl(%s) = %s\n", options[i].str, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl")

#endif
