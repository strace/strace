/*
 * Check decoding of fadvise64_64 syscall.
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

#ifdef __arm__
# ifdef __NR_arm_fadvise64_64
#  undef __NR_fadvise64_64
#  define __NR_fadvise64_64 __NR_arm_fadvise64_64
# endif /* __NR_arm_fadvise64_64 */
#endif /* __arm__ */

#ifdef __NR_fadvise64_64

# include "fadvise.h"

static void
do_fadvise(long fd, long long offset, long long llen, long advice)
{
	long ret;
	const char *errstr;

# if (LONG_MAX > INT_MAX) \
  || (defined __x86_64__ && defined __ILP32__) \
  || defined LINUX_MIPSN32
	ret = syscall(__NR_fadvise64_64, fd, offset, llen, advice);
# elif defined __ARM_EABI__ || defined POWERPC || defined XTENSA
	ret = syscall(__NR_fadvise64_64, fd, advice,
		      LL_VAL_TO_PAIR(offset), LL_VAL_TO_PAIR(llen));
# else
	ret = syscall(__NR_fadvise64_64, fd,
		      LL_VAL_TO_PAIR(offset), LL_VAL_TO_PAIR(llen), advice);
# endif
	errstr = sprintrc(ret);

	printf("fadvise64_64(%d, %lld, %lld, ", (int) fd, offset, llen);
	printxval(advise, (unsigned) advice, "POSIX_FADV_???");
	printf(") = %s\n", errstr);
}

#else

SKIP_MAIN_UNDEFINED("__NR_fadvise64_64");

#endif
