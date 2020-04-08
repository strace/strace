/*
 * This file is part of rt_tgsigqueueinfo strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_rt_tgsigqueueinfo

# include <errno.h>
# include <signal.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

static long
k_tgsigqueueinfo(const pid_t pid, const int sig, const void *const info)
{
	return syscall(__NR_rt_tgsigqueueinfo,
		       F8ILL_KULONG_MASK | pid,
		       F8ILL_KULONG_MASK | pid,
		       F8ILL_KULONG_MASK | sig,
		       info);
}

int
main(void)
{
	const struct sigaction sa = {
		.sa_handler = SIG_IGN
	};
	if (sigaction(SIGUSR1, &sa, NULL))
		perror_msg_and_fail("sigaction");

	TAIL_ALLOC_OBJECT_CONST_PTR(siginfo_t, info);
	memset(info, 0, sizeof(*info));
	info->si_signo = SIGUSR1;
	info->si_errno = ENOENT;
	info->si_code = SI_QUEUE;
	info->si_pid = getpid();
	info->si_uid = getuid();
	info->si_value.sival_ptr =
		(void *) (unsigned long) 0xdeadbeeffacefeedULL;

	if (k_tgsigqueueinfo(info->si_pid, SIGUSR1, info))
		(errno == ENOSYS ? perror_msg_and_skip : perror_msg_and_fail)(
			"rt_tgsigqueueinfo");

	printf("rt_tgsigqueueinfo(%u, %u, %s, {si_signo=%s"
		", si_code=SI_QUEUE, si_errno=ENOENT, si_pid=%u"
		", si_uid=%u, si_value={int=%d, ptr=%p}}) = 0\n",
		info->si_pid, info->si_pid, "SIGUSR1", "SIGUSR1",
		info->si_pid, info->si_uid, info->si_value.sival_int,
		info->si_value.sival_ptr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_rt_tgsigqueueinfo")

#endif
