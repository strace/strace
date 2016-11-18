/*
 * Check decoding of ERESTARTSYS error code.
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

#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>

static int sv[2];

static void
handler(int sig)
{
	close(sv[1]);
	sv[1] = -1;
}

int
main(void)
{
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
		perror_msg_and_skip("socketpair");

	const struct sigaction act = {
		.sa_handler = handler,
		.sa_flags = SA_RESTART
	};
	if (sigaction(SIGALRM, &act, NULL))
		perror_msg_and_fail("sigaction");

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	const struct itimerval itv = { .it_value.tv_usec = 123456 };
	if (setitimer(ITIMER_REAL, &itv, NULL))
		perror_msg_and_fail("setitimer");

	if (recvfrom(sv[0], &sv[1], sizeof(sv[1]), 0, NULL, NULL))
		perror_msg_and_fail("recvfrom");

	printf("recvfrom(%d, %p, %d, 0, NULL, NULL) = ? ERESTARTSYS"
	       " (To be restarted if SA_RESTART is set)\n",
	       sv[0], &sv[1], (int) sizeof(sv[1]));
	printf("recvfrom(%d, \"\", %d, 0, NULL, NULL) = 0\n",
	       sv[0], (int) sizeof(sv[1]));

	puts("+++ exited with 0 +++");
	return 0;
}
