/*
 * Check decoding of signal delivery.
 *
 * Copyright (c) 2016-2018 The strace developers.
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
#include <unistd.h>

static volatile int s_sig, s_code, s_pid, s_uid;

static void
handler(int sig, siginfo_t *info, void *ucontext)
{
	s_sig = info->si_signo;
	s_code = info->si_code;
	s_pid = info->si_pid;
	s_uid = info->si_uid;
}

int
main(void)
{
	static const char prefix[] = "KERNEL BUG";
	int printed = 0;

	const int pid = getpid();
	const int uid = geteuid();

	for (int sig = 1; sig <= 31; ++sig) {
		if (sig == SIGKILL || sig == SIGSTOP)
			continue;

		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, sig);
		if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
			perror_msg_and_fail("sigprocmask");

		static const struct sigaction act = {
			.sa_sigaction = handler,
			.sa_flags = SA_SIGINFO
		};
		if (sigaction(sig, &act, NULL))
			perror_msg_and_fail("sigaction: %d", sig);

		if (kill(pid, sig) != 0)
			perror_msg_and_fail("kill: %d", sig);

#ifdef MPERS_IS_m32
		/*
		 * The tracee has received a compat siginfo_t but
		 * the tracer has received a native siginfo_t.
		 */
		const int e_sig = sig;
		const int e_pid = pid;
		const int e_uid = uid;
#else
		/*
		 * If the tracee is a native process,
		 * then the tracer is also native.
		 * If the tracee is a compat process,
		 * then the tracer is also compat.
		 * Anyway, both the tracee and the tracer
		 * have received the same siginfo_t.
		 */
		const int e_sig = s_sig;
		const int e_pid = s_pid;
		const int e_uid = s_uid;
#endif
		printf("kill(%d, %s) = 0\n", pid, signal2name(sig));
		printf("--- %s {si_signo=%s, si_code=SI_USER, si_pid=%d"
		       ", si_uid=%d} ---\n",
		       signal2name(sig), signal2name(e_sig), e_pid, e_uid);

		if (s_code || sig != s_sig || pid != s_pid || uid != s_uid) {
			/*
			 * The kernel has failed to initialize siginfo_t
			 * properly.  There is nothing that could be done
			 * on the strace side to workaround the kernel bug,
			 * so just print some useful diagnostics.
			 */
			if (!printed) {
				printed = 1;
				fprintf(stderr, "%s: siginfo_t\n", prefix);
			}
			fprintf(stderr,
				"%s: expected: si_signo=%d, si_code=%d"
				", si_pid=%d, si_uid=%d\n"
				"%s: received: si_signo=%d, si_code=%d"
				", si_pid=%d, si_uid=%d\n",
				prefix, sig, SI_USER, pid, uid,
				prefix, sig, s_code, s_pid, s_uid);
		}
	}

	if (printed) {
		fprintf(stderr, "%s: end of diagnostics\n"
			"*** PLEASE FIX THE KERNEL ***\n", prefix);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
