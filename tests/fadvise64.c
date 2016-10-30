/*
 * Check decoding of fadvise64 syscall.
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

#ifdef __NR_fadvise64

# include "fadvise.h"

static void
do_fadvise(long fd, long long offset, long long llen, long advice)
{
	long ret;
	const char *errstr;

# if (LONG_MAX > INT_MAX) \
  || (defined __x86_64__ && defined __ILP32__) \
  || defined LINUX_MIPSN32
	ret = syscall(__NR_fadvise64, fd, offset, llen, advice);
	errstr = sprintrc(ret);
	printf("fadvise64(%d, %lld, %llu, ", (int) fd, offset, llen);
# elif defined LINUX_MIPSO32
	ret = syscall(__NR_fadvise64, fd, 0,
		      LL_VAL_TO_PAIR(offset), LL_VAL_TO_PAIR(llen), advice);
	errstr = sprintrc(ret);
	printf("fadvise64(%d, %lld, %lld, ", (int) fd, offset, llen);
# else /* LONG_MAX == INT_MAX && !X32 && !LINUX_MIPSN32 */
	long len = (long) llen;
#  if defined POWERPC
	ret = syscall(__NR_fadvise64, fd, 0,
		      LL_VAL_TO_PAIR(offset), len, advice);
#  else
	ret = syscall(__NR_fadvise64, fd,
		      LL_VAL_TO_PAIR(offset), len, advice);
#  endif
	errstr = sprintrc(ret);
	printf("fadvise64(%d, %lld, %lu, ", (int) fd, offset, len);
# endif /* LONG_MAX == INT_MAX && !X32 && !LINUX_MIPSN32 */
	printxval(advise, (unsigned) advice, "POSIX_FADV_???");
	printf(") = %s\n", errstr);
}

#else

SKIP_MAIN_UNDEFINED("__NR_fadvise64");

#endif
