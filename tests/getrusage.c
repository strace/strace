/*
 * Copyright (c) 2016 Fei Jie <feij.fnst@cn.fujitsu.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_getrusage

# include <stdio.h>
# include <stdint.h>
# include <sys/resource.h>
# include <unistd.h>
# include <errno.h>

# include "kernel_rusage.h"
# include "xlat.h"
# include "xlat/usagewho.h"

int
invoke_print(int who, const char *who_str, kernel_rusage_t *usage)
{
	int rc = syscall(__NR_getrusage, who, usage);
	int saved_errno = errno;
	printf("getrusage(%s, {ru_utime={tv_sec=%lld, tv_usec=%llu}"
	       ", ru_stime={tv_sec=%lld, tv_usec=%llu}, ru_maxrss=%llu"
	       ", ru_ixrss=%llu, ru_idrss=%llu, ru_isrss=%llu, ru_minflt=%llu"
	       ", ru_majflt=%llu, ru_nswap=%llu, ru_inblock=%llu"
	       ", ru_oublock=%llu, ru_msgsnd=%llu, ru_msgrcv=%llu"
	       ", ru_nsignals=%llu, ru_nvcsw=%llu, ru_nivcsw=%llu}) = %s\n",
	       who_str,
	       (long long) usage->ru_utime.tv_sec,
	       zero_extend_signed_to_ull(usage->ru_utime.tv_usec),
	       (long long) usage->ru_stime.tv_sec,
	       zero_extend_signed_to_ull(usage->ru_stime.tv_usec),
	       zero_extend_signed_to_ull(usage->ru_maxrss),
	       zero_extend_signed_to_ull(usage->ru_ixrss),
	       zero_extend_signed_to_ull(usage->ru_idrss),
	       zero_extend_signed_to_ull(usage->ru_isrss),
	       zero_extend_signed_to_ull(usage->ru_minflt),
	       zero_extend_signed_to_ull(usage->ru_majflt),
	       zero_extend_signed_to_ull(usage->ru_nswap),
	       zero_extend_signed_to_ull(usage->ru_inblock),
	       zero_extend_signed_to_ull(usage->ru_oublock),
	       zero_extend_signed_to_ull(usage->ru_msgsnd),
	       zero_extend_signed_to_ull(usage->ru_msgrcv),
	       zero_extend_signed_to_ull(usage->ru_nsignals),
	       zero_extend_signed_to_ull(usage->ru_nvcsw),
	       zero_extend_signed_to_ull(usage->ru_nivcsw),
	       sprintrc(rc));
	errno = saved_errno;
	return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_rusage_t, usage);
	if (invoke_print(ARG_STR(RUSAGE_SELF), usage)) {
		perror_msg_and_fail("RUSAGE_SELF");
	}
	if (invoke_print(ARG_STR(RUSAGE_THREAD), usage) && errno != EINVAL) {
		perror_msg_and_fail("RUSAGE_THREAD");
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getrusage")

#endif
