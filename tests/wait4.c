/*
 * Check decoding of wait4 syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_wait4

# include <assert.h>
# include <signal.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/wait.h>
# include "kernel_rusage.h"

static const char *
sprint_rusage(const kernel_rusage_t *const ru)
{
	static char buf[1024];
	snprintf(buf, sizeof(buf),
		 "{ru_utime={tv_sec=%llu, tv_usec=%llu}"
		 ", ru_stime={tv_sec=%llu, tv_usec=%llu}"
# if VERBOSE
		 ", ru_maxrss=%llu"
		 ", ru_ixrss=%llu"
		 ", ru_idrss=%llu"
		 ", ru_isrss=%llu"
		 ", ru_minflt=%llu"
		 ", ru_majflt=%llu"
		 ", ru_nswap=%llu"
		 ", ru_inblock=%llu"
		 ", ru_oublock=%llu"
		 ", ru_msgsnd=%llu"
		 ", ru_msgrcv=%llu"
		 ", ru_nsignals=%llu"
		 ", ru_nvcsw=%llu"
		 ", ru_nivcsw=%llu}"
# else
		 ", ...}"
# endif
		 , zero_extend_signed_to_ull(ru->ru_utime.tv_sec)
		 , zero_extend_signed_to_ull(ru->ru_utime.tv_usec)
		 , zero_extend_signed_to_ull(ru->ru_stime.tv_sec)
		 , zero_extend_signed_to_ull(ru->ru_stime.tv_usec)
# if VERBOSE
		 , zero_extend_signed_to_ull(ru->ru_maxrss)
		 , zero_extend_signed_to_ull(ru->ru_ixrss)
		 , zero_extend_signed_to_ull(ru->ru_idrss)
		 , zero_extend_signed_to_ull(ru->ru_isrss)
		 , zero_extend_signed_to_ull(ru->ru_minflt)
		 , zero_extend_signed_to_ull(ru->ru_majflt)
		 , zero_extend_signed_to_ull(ru->ru_nswap)
		 , zero_extend_signed_to_ull(ru->ru_inblock)
		 , zero_extend_signed_to_ull(ru->ru_oublock)
		 , zero_extend_signed_to_ull(ru->ru_msgsnd)
		 , zero_extend_signed_to_ull(ru->ru_msgrcv)
		 , zero_extend_signed_to_ull(ru->ru_nsignals)
		 , zero_extend_signed_to_ull(ru->ru_nvcsw)
		 , zero_extend_signed_to_ull(ru->ru_nivcsw)
# endif
		 );
	return buf;
}

static const char *errstr;

static long
k_wait4(const unsigned int pid, void const *wstatus,
	const unsigned int options, void const *ru)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | pid;
	const kernel_ulong_t arg2 = (uintptr_t) wstatus;
	const kernel_ulong_t arg3 = fill | options;
	const kernel_ulong_t arg4 = (uintptr_t) ru;
	const long rc = syscall(__NR_wait4, arg1, arg2, arg3, arg4, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

static pid_t
do_wait4(pid_t pid, int *wstatus, int options, kernel_rusage_t *ru)
{
	sigset_t mask = {};
	sigaddset(&mask, SIGCHLD);

	assert(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);
	pid_t rc = k_wait4(pid, wstatus, options, ru);
	assert(sigprocmask(SIG_UNBLOCK, &mask, NULL) == 0);
	return rc;
}

int
main(void)
{
	tprintf("%s", "");

	int fds[2];
	if (pipe(fds))
		perror_msg_and_fail("pipe");

	pid_t pid;
	pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		char c;
		(void) close(1);
		assert(read(0, &c, sizeof(c)) == 1);
		return 42;
	}

	(void) close(0);

	TAIL_ALLOC_OBJECT_CONST_PTR(int, s);
	if (k_wait4(pid, s, WNOHANG|__WALL, NULL))
		perror_msg_and_fail("wait4 #1");
	tprintf("wait4(%d, %p, WNOHANG|__WALL, NULL) = 0\n", pid, s);

	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_rusage_t, rusage);
	if (k_wait4(pid, s, WNOHANG|__WALL, rusage))
		perror_msg_and_fail("wait4 #2");
	tprintf("wait4(%d, %p, WNOHANG|__WALL, %p) = 0\n", pid, s, rusage);

	assert(write(1, "", 1) == 1);
	(void) close(1);

	assert(do_wait4(pid, s, 0, rusage) == pid);
	assert(WIFEXITED(*s) && WEXITSTATUS(*s) == 42);
	tprintf("wait4(%d, [{WIFEXITED(s) && WEXITSTATUS(s) == 42}], 0, %s)"
		" = %d\n", pid, sprint_rusage(rusage), pid);

	pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		(void) raise(SIGUSR1);
		return 1;
	}

	assert(do_wait4(pid, s, __WALL, rusage) == pid);
	assert(WIFSIGNALED(*s) && WTERMSIG(*s) == SIGUSR1);
	tprintf("wait4(%d, [{WIFSIGNALED(s) && WTERMSIG(s) == SIGUSR1}]"
		", __WALL, %s) = %d\n", pid, sprint_rusage(rusage), pid);

	if (pipe(fds))
		perror_msg_and_fail("pipe");
	pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		(void) close(1);
		raise(SIGSTOP);
		char c;
		assert(read(0, &c, sizeof(c)) == 1);
		return 0;
	}

	(void) close(0);

	assert(do_wait4(pid, s, WSTOPPED, rusage) == pid);
	assert(WIFSTOPPED(*s) && WSTOPSIG(*s) == SIGSTOP);
	tprintf("wait4(%d, [{WIFSTOPPED(s) && WSTOPSIG(s) == SIGSTOP}]"
		", WSTOPPED, %s) = %d\n", pid, sprint_rusage(rusage), pid);

	if (kill(pid, SIGCONT))
		perror_msg_and_fail("kill(SIGCONT)");

# if defined WCONTINUED && defined WIFCONTINUED
	assert(do_wait4(pid, s, WCONTINUED, rusage) == pid);
	assert(WIFCONTINUED(*s));
	tprintf("wait4(%d, [{WIFCONTINUED(s)}], WCONTINUED"
		", %s) = %d\n", pid, sprint_rusage(rusage), pid);
# endif /* WCONTINUED && WIFCONTINUED */

	assert(write(1, "", 1) == 1);
	(void) close(1);

	assert(do_wait4(pid, s, 0, rusage) == pid);
	assert(WIFEXITED(*s) && WEXITSTATUS(*s) == 0);
	tprintf("wait4(%d, [{WIFEXITED(s) && WEXITSTATUS(s) == 0}], 0"
		", %s) = %d\n", pid, sprint_rusage(rusage), pid);

	assert(k_wait4(-1, s, WNOHANG|WSTOPPED|__WALL, rusage) == -1);
	tprintf("wait4(-1, %p, WNOHANG|WSTOPPED|__WALL, %p) = %s\n",
		s, rusage, errstr);

	tprintf("%s\n", "+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_wait4")

#endif /* __NR_wait4 */
