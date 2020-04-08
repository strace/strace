/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <signal.h>

#define CASE(x) case x: return #x

const char *
signal2name(int sig)
{
	switch (sig) {
	CASE(SIGALRM);
	CASE(SIGBUS);
	CASE(SIGCHLD);
	CASE(SIGCONT);
	CASE(SIGFPE);
	CASE(SIGHUP);
	CASE(SIGILL);
	CASE(SIGINT);
	CASE(SIGIO);
	CASE(SIGPIPE);
	CASE(SIGPROF);
	CASE(SIGQUIT);
	CASE(SIGSEGV);
	CASE(SIGSYS);
	CASE(SIGTERM);
	CASE(SIGTRAP);
	CASE(SIGTSTP);
	CASE(SIGTTIN);
	CASE(SIGTTOU);
	CASE(SIGURG);
	CASE(SIGUSR1);
	CASE(SIGUSR2);
	CASE(SIGVTALRM);
	CASE(SIGWINCH);
	CASE(SIGXCPU);
	CASE(SIGXFSZ);
#if defined ALPHA
	CASE(SIGABRT);
	CASE(SIGEMT);
	CASE(SIGINFO);
#elif defined SPARC || defined SPARC64
	CASE(SIGABRT);
	CASE(SIGEMT);
	CASE(SIGLOST);
#elif defined MIPS
	CASE(SIGEMT);
	CASE(SIGIOT);
	CASE(SIGPWR);
#else
	CASE(SIGABRT);
	CASE(SIGPWR);
	CASE(SIGSTKFLT);
#endif
	default:
		perror_msg_and_fail("unknown signal number %d", sig);
	}
}
