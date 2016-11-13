/*
 * Check decoding of kill syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Fei Jie <feij.fnst@cn.fujitsu.com>
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

#ifdef __NR_kill

# include <signal.h>
# include <stdio.h>
# include <unistd.h>

static void
handler(int sig)
{
}

int
main(void)
{
	const struct sigaction act = { .sa_handler = handler };
	if (sigaction(SIGALRM, &act, NULL))
		perror_msg_and_fail("sigaction");

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	const int pid = getpid();
	long rc = syscall(__NR_kill, pid, (long) 0xdefaced00000000ULL | SIGALRM);
	printf("kill(%d, SIGALRM) = %ld\n", pid, rc);

	const long big_pid = (long) 0xfacefeedbadc0dedULL;
	const long big_sig = (long) 0xdeadbeefcafef00dULL;
	rc = syscall(__NR_kill, big_pid, big_sig);
	printf("kill(%d, %d) = %ld %s (%m)\n",
	       (int) big_pid, (int) big_sig, rc, errno2name());

	rc = syscall(__NR_kill, (long) 0xdefaced00000000ULL | pid, 0);
	printf("kill(%d, SIG_0) = %ld\n", pid, rc);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_kill")

#endif
