/*
 * Check decoding of unshare syscall.
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

#ifdef __NR_unshare

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const kernel_ulong_t bogus_flags =
		(kernel_ulong_t) 0xbadc0ded0000000fULL;

	static struct {
		kernel_ulong_t val;
		const char *str;
	} unshare_flags[] = {
		{ ARG_STR(0) },
		{ 0xdeadca75,
			"CLONE_THREAD|CLONE_FS|CLONE_SIGHAND|CLONE_SYSVSEM|"
			"CLONE_NEWUTS|CLONE_NEWIPC|CLONE_NEWNET|CLONE_NEWUSER|"
			"CLONE_NEWCGROUP|0x80a8c075" },
		{ 0x2000000, "CLONE_NEWCGROUP" },
		{ ARG_STR(0x81f8f0ff) " /* CLONE_??? */" },
	};

	long rc;
	unsigned int i;

	rc = syscall(__NR_unshare, bogus_flags);
	printf("unshare(%#llx /* CLONE_??? */) = %s\n",
	       (unsigned long long) bogus_flags, sprintrc(rc));

	for (i = 0; i < ARRAY_SIZE(unshare_flags); i++) {
		rc = syscall(__NR_unshare, unshare_flags[i].val);
		printf("unshare(%s) = %s\n",
		       unshare_flags[i].str, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_unshare");

#endif
