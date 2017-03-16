/*
 * This file is part of rt_tgsigqueueinfo strace test.
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
main (void)
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
	info->si_value.sival_ptr = (void *) (unsigned long) 0xdeadbeeffacefeedULL;

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
