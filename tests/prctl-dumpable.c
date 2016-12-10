/*
 * Check decoding of prctl PR_GET_DUMPABLE/PR_SET_DUMPABLE operations.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <linux/prctl.h>

#if defined __NR_prctl && defined PR_GET_DUMPABLE && defined PR_SET_DUMPABLE

# include <stdio.h>
# include <unistd.h>

static const char *errstr;

static long
prctl(kernel_ulong_t arg1, kernel_ulong_t arg2)
{
	static const kernel_ulong_t bogus_arg =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;
	long rc = syscall(__NR_prctl, arg1, arg2, bogus_arg);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const kernel_ulong_t bogus_dumpable1 =
		(kernel_ulong_t) 0xdeadc0de00000001ULL;
	static const kernel_ulong_t bogus_dumpable2 =
		(kernel_ulong_t) 0xdeadc0defacebeefULL;

	static const char * const args[] = {
		"SUID_DUMP_DISABLE",
		"SUID_DUMP_USER",
		"SUID_DUMP_ROOT",
	};

	unsigned int i;

	prctl(PR_SET_DUMPABLE, 3);
	printf("prctl(PR_SET_DUMPABLE, 0x3 /* SUID_DUMP_??? */) = %s\n",
	       errstr);

	prctl(PR_SET_DUMPABLE, bogus_dumpable1);
	if (bogus_dumpable1 == 1) {
		printf("prctl(PR_SET_DUMPABLE, SUID_DUMP_USER) = %s\n", errstr);
	} else {
		printf("prctl(PR_SET_DUMPABLE, %#llx /* SUID_DUMP_??? */)"
		       " = %s\n",
		       (unsigned long long) bogus_dumpable1, errstr);
	}

	prctl(PR_SET_DUMPABLE, bogus_dumpable2);
	printf("prctl(PR_SET_DUMPABLE, %#llx /* SUID_DUMP_??? */) = %s\n",
	       (unsigned long long) bogus_dumpable2, errstr);

	for (i = 0; i < ARRAY_SIZE(args); ++i) {
		prctl(PR_SET_DUMPABLE, i);
		printf("prctl(PR_SET_DUMPABLE, %s) = %s\n", args[i], errstr);

		long rc = prctl(PR_GET_DUMPABLE, bogus_dumpable2);
		if (rc >= 0 && rc < (long) ARRAY_SIZE(args)) {
			printf("prctl(PR_GET_DUMPABLE) = %s (%s)\n",
			       errstr, args[rc]);
		} else {
			printf("prctl(PR_GET_DUMPABLE) = %s\n", errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl && PR_GET_DUMPABLE && PR_SET_DUMPABLE")

#endif
