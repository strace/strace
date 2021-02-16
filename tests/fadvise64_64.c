/*
 * Check decoding of fadvise64_64 syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

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
