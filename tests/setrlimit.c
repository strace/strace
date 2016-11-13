/*
 * Check decoding of setrlimit syscall.
 *
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

#ifdef __NR_setrlimit

# include "xgetrlimit.c"

int
main(void)
{
	kernel_ulong_t *const rlimit = tail_alloc(sizeof(*rlimit) * 2);
	const struct xlat *xlat;

	for (xlat = resources; xlat->str; ++xlat) {
		unsigned long res = 0xfacefeed00000000ULL | xlat->val;
		long rc = syscall(__NR_setrlimit, res, 0);
		printf("setrlimit(%s, NULL) = %s\n", xlat->str, sprintrc(rc));

		struct rlimit libc_rlim = {};
		if (getrlimit((int) res, &libc_rlim))
			continue;
		rlimit[0] = libc_rlim.rlim_cur;
		rlimit[1] = libc_rlim.rlim_max;

		rc = syscall(__NR_setrlimit, res, rlimit);
		const char *errstr = sprintrc(rc);
		printf("setrlimit(%s, {rlim_cur=%s, rlim_max=%s}) = %s\n",
		       xlat->str,
		       sprint_rlim(rlimit[0]), sprint_rlim(rlimit[1]),
		       errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_setrlimit")

#endif
