/*
 * Check decoding of fadvise64 syscall.
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
