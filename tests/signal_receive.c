/*
 * Check decoding of signal delivery.
 *
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "pidns.h"
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
	PIDNS_TEST_INIT;

	static const char prefix[] = "KERNEL BUG";
	int printed = 0;

	const int pid = getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);
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
		pidns_print_leader();
		printf("kill(%d%s, %s) = 0\n", pid, pid_str, signal2name(sig));
		pidns_print_leader();
		printf("--- %s {si_signo=%s, si_code=SI_USER, si_pid=%d%s"
		       ", si_uid=%d} ---\n",
		       signal2name(sig), signal2name(e_sig),
		       e_pid, pid_str, e_uid);

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
				", si_pid=%d%s, si_uid=%d\n"
				"%s: received: si_signo=%d, si_code=%d"
				", si_pid=%d%s, si_uid=%d\n",
				prefix, sig, SI_USER, pid, pid_str, uid,
				prefix, sig, s_code, s_pid, pid_str, s_uid);
		}
	}

	if (printed) {
		fprintf(stderr, "%s: end of diagnostics\n"
			"*** PLEASE FIX THE KERNEL ***\n", prefix);
	}

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
