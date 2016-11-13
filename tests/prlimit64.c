/*
 * Check decoding of prlimit64 syscall.
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

#ifdef __NR_prlimit64

# include <inttypes.h>
# include <stdio.h>
# include <stdint.h>
# include <sys/resource.h>
# include <unistd.h>

# include "xlat.h"
# include "xlat/resources.h"

const char *
sprint_rlim(uint64_t lim)
{
	if (lim == -1ULL)
		return "RLIM64_INFINITY";

	static char buf[2][sizeof(lim)*3 + sizeof("*1024")];
	static int i;
	i &= 1;
	if (lim > 1024 && lim % 1024 == 0)
		sprintf(buf[i], "%" PRIu64 "*1024", lim / 1024);
	else
		sprintf(buf[i], "%" PRIu64, lim);

	return buf[i++];
}

int
main(void)
{
	unsigned long pid =
		(unsigned long) 0xdefaced00000000ULL | (unsigned) getpid();
	uint64_t *const rlimit = tail_alloc(sizeof(*rlimit) * 2);
	const struct xlat *xlat;

	for (xlat = resources; xlat->str; ++xlat) {
		unsigned long res = 0xfacefeed00000000ULL | xlat->val;
		long rc = syscall(__NR_prlimit64, pid, res, 0, rlimit);
		if (rc)
			printf("prlimit64(%d, %s, NULL, %p) = %ld %s (%m)\n",
			       (unsigned) pid, xlat->str, rlimit,
			       rc, errno2name());
		else
			printf("prlimit64(%d, %s, NULL"
			       ", {rlim_cur=%s, rlim_max=%s}) = 0\n",
			       (unsigned) pid, xlat->str,
			       sprint_rlim(rlimit[0]),
			       sprint_rlim(rlimit[1]));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prlimit64")

#endif
