/*
 * Copyright (c) 2016 Fei Jie <feij.fnst@cn.fujitsu.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
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

# include "xlat.h"
# include "xlat/usagewho.h"

int
invoke_print(int who, const char *who_str, struct rusage *usage)
{
	int rc = syscall(__NR_getrusage, who, usage);
	int saved_errno = errno;
	printf("getrusage(%s, {ru_utime={tv_sec=%lld, tv_usec=%llu}"
	       ", ru_stime={tv_sec=%lld, tv_usec=%llu}, ru_maxrss=%lu"
	       ", ru_ixrss=%lu, ru_idrss=%lu, ru_isrss=%lu, ru_minflt=%lu"
	       ", ru_majflt=%lu, ru_nswap=%lu, ru_inblock=%lu"
	       ", ru_oublock=%lu, ru_msgsnd=%lu, ru_msgrcv=%lu"
	       ", ru_nsignals=%lu, ru_nvcsw=%lu, ru_nivcsw=%lu}) = %s\n",
	       who_str,
	       (long long) usage->ru_utime.tv_sec,
	       zero_extend_signed_to_ull(usage->ru_utime.tv_usec),
	       (long long) usage->ru_stime.tv_sec,
	       zero_extend_signed_to_ull(usage->ru_stime.tv_usec),
	       usage->ru_maxrss, usage->ru_ixrss, usage->ru_idrss,
	       usage->ru_isrss, usage->ru_minflt, usage->ru_majflt,
	       usage->ru_nswap, usage->ru_inblock, usage->ru_oublock,
	       usage->ru_msgsnd, usage->ru_msgrcv, usage->ru_nsignals,
	       usage->ru_nvcsw, usage->ru_nivcsw, sprintrc(rc));
	errno = saved_errno;
	return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct rusage, usage);
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
