/*
 * Check decoding of waitid syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include "scno.h"

/* Workaround for glibc/kernel interface discrepancy */
#ifdef HAVE_STRUCT_RUSAGE___RU_MAXRSS_WORD
# define RU_MAXRSS __ru_maxrss_word
#else
# define RU_MAXRSS ru_maxrss
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_IXRSS_WORD
# define RU_IXRSS __ru_ixrss_word
#else
# define RU_IXRSS ru_ixrss
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_IDRSS_WORD
# define RU_IDRSS __ru_idrss_word
#else
# define RU_IDRSS ru_idrss
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_ISRSS_WORD
# define RU_ISRSS __ru_isrss_word
#else
# define RU_ISRSS ru_isrss
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_MINFLT_WORD
# define RU_MINFLT __ru_minflt_word
#else
# define RU_MINFLT ru_minflt
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_MAJFLT_WORD
# define RU_MAJFLT __ru_majflt_word
#else
# define RU_MAJFLT ru_majflt
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_NSWAP_WORD
# define RU_NSWAP __ru_nswap_word
#else
# define RU_NSWAP ru_nswap
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_INBLOCK_WORD
# define RU_INBLOCK __ru_inblock_word
#else
# define RU_INBLOCK ru_inblock
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_OUBLOCK_WORD
# define RU_OUBLOCK __ru_oublock_word
#else
# define RU_OUBLOCK ru_oublock
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_MSGSND_WORD
# define RU_MSGSND __ru_msgsnd_word
#else
# define RU_MSGSND ru_msgsnd
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_MSGRCV_WORD
# define RU_MSGRCV __ru_msgrcv_word
#else
# define RU_MSGRCV ru_msgrcv
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_NSIGNALS_WORD
# define RU_NSIGNALS __ru_nsignals_word
#else
# define RU_NSIGNALS ru_nsignals
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_NVCSW_WORD
# define RU_NVCSW __ru_nvcsw_word
#else
# define RU_NVCSW ru_nvcsw
#endif
#ifdef HAVE_STRUCT_RUSAGE___RU_NIVCSW_WORD
# define RU_NIVCSW __ru_nivcsw_word
#else
# define RU_NIVCSW ru_nivcsw
#endif

static const char *
sprint_rusage(const struct rusage *const ru)
{
	static char buf[1024];
	snprintf(buf, sizeof(buf),
		 "{ru_utime={tv_sec=%lld, tv_usec=%llu}"
		 ", ru_stime={tv_sec=%lld, tv_usec=%llu}"
#if VERBOSE
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
#else
		 ", ...}"
#endif
		 , (long long) ru->ru_utime.tv_sec
		 , zero_extend_signed_to_ull(ru->ru_utime.tv_usec)
		 , (long long) ru->ru_stime.tv_sec
		 , zero_extend_signed_to_ull(ru->ru_stime.tv_usec)
#if VERBOSE
		 , zero_extend_signed_to_ull(ru->RU_MAXRSS)
		 , zero_extend_signed_to_ull(ru->RU_IXRSS)
		 , zero_extend_signed_to_ull(ru->RU_IDRSS)
		 , zero_extend_signed_to_ull(ru->RU_ISRSS)
		 , zero_extend_signed_to_ull(ru->RU_MINFLT)
		 , zero_extend_signed_to_ull(ru->RU_MAJFLT)
		 , zero_extend_signed_to_ull(ru->RU_NSWAP)
		 , zero_extend_signed_to_ull(ru->RU_INBLOCK)
		 , zero_extend_signed_to_ull(ru->RU_OUBLOCK)
		 , zero_extend_signed_to_ull(ru->RU_MSGSND)
		 , zero_extend_signed_to_ull(ru->RU_MSGRCV)
		 , zero_extend_signed_to_ull(ru->RU_NSIGNALS)
		 , zero_extend_signed_to_ull(ru->RU_NVCSW)
		 , zero_extend_signed_to_ull(ru->RU_NIVCSW)
#endif
		 );
	return buf;
}

#define CASE(x) case x: return #x

static const char *
si_code_2_name(const int code)
{
	switch (code) {
#ifdef CLD_EXITED
	CASE(CLD_EXITED);
#endif
#ifdef CLD_KILLED
	CASE(CLD_KILLED);
#endif
#ifdef CLD_DUMPED
	CASE(CLD_DUMPED);
#endif
#ifdef CLD_TRAPPED
	CASE(CLD_TRAPPED);
#endif
#ifdef CLD_STOPPED
	CASE(CLD_STOPPED);
#endif
#ifdef CLD_CONTINUED
	CASE(CLD_CONTINUED);
#endif
	default:
		perror_msg_and_fail("unknown si_code %d", code);
	}
}

static const char *
sprint_siginfo(const siginfo_t *const si, const char *const status_text)
{
	static char buf[1024];
	snprintf(buf, sizeof(buf),
		 "{si_signo=SIGCHLD"
		 ", si_code=%s"
		 ", si_pid=%u"
		 ", si_uid=%u"
		 ", si_status=%s"
		 ", si_utime=%llu"
		 ", si_stime=%llu}",
		 si_code_2_name(si->si_code),
		 si->si_pid,
		 si->si_uid,
		 status_text,
		 zero_extend_signed_to_ull(si->si_utime),
		 zero_extend_signed_to_ull(si->si_stime));
	return buf;
}

static unsigned long
poison(unsigned int v)
{
	return (unsigned long) 0xfacefeed00000000ULL | v;
}

static long
do_waitid(const unsigned int idtype,
	  const unsigned int id,
	  const siginfo_t *const infop,
	  const unsigned int options,
	  const struct rusage *const rusage)
{
	sigset_t mask = {};
	sigaddset(&mask, SIGCHLD);

	assert(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);
	long rc = syscall(__NR_waitid, poison(idtype), poison(id),
			  infop, poison(options), rusage);
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

	if (do_waitid(P_PID, pid, 0, WNOHANG|WEXITED, 0))
		perror_msg_and_fail("waitid #1");
	tprintf("waitid(P_PID, %d, NULL, WNOHANG|WEXITED, NULL) = 0\n", pid);

	TAIL_ALLOC_OBJECT_CONST_PTR(siginfo_t, sinfo);
	memset(sinfo, 0, sizeof(*sinfo));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct rusage, rusage);
	if (do_waitid(P_PID, pid, sinfo, WNOHANG|WEXITED|WSTOPPED, rusage))
		perror_msg_and_fail("waitid #2");
	tprintf("waitid(P_PID, %d, {}, WNOHANG|WEXITED|WSTOPPED, %s) = 0\n",
		pid, sprint_rusage(rusage));

	assert(write(1, "", 1) == 1);
	(void) close(1);

	if (do_waitid(P_PID, pid, sinfo, WEXITED, rusage))
		perror_msg_and_fail("waitid #3");
	tprintf("waitid(P_PID, %d, %s, WEXITED, %s) = 0\n",
		pid, sprint_siginfo(sinfo, "42"), sprint_rusage(rusage));

	pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		(void) raise(SIGUSR1);
		return 1;
	}

	if (do_waitid(P_PID, pid, sinfo, WEXITED, rusage))
		perror_msg_and_fail("waitid #4");
	tprintf("waitid(P_PID, %d, %s, WEXITED, %s) = 0\n",
		pid, sprint_siginfo(sinfo, "SIGUSR1"), sprint_rusage(rusage));

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

	if (do_waitid(P_PID, pid, sinfo, WSTOPPED, rusage))
		perror_msg_and_fail("waitid #5");
	tprintf("waitid(P_PID, %d, %s, WSTOPPED, %s) = 0\n",
		pid, sprint_siginfo(sinfo, "SIGSTOP"), sprint_rusage(rusage));

	if (kill(pid, SIGCONT))
		perror_msg_and_fail("kill(SIGCONT)");

#if defined WCONTINUED
	if (do_waitid(P_PID, pid, sinfo, WCONTINUED, rusage))
		perror_msg_and_fail("waitid #6");
	tprintf("waitid(P_PID, %d, %s, WCONTINUED, %s) = 0\n",
		pid, sprint_siginfo(sinfo, "SIGCONT"), sprint_rusage(rusage));
#endif /* WCONTINUED */

	assert(write(1, "", 1) == 1);
	(void) close(1);

	if (do_waitid(P_PID, pid, sinfo, WEXITED, rusage))
		perror_msg_and_fail("waitid #7");
	tprintf("waitid(P_PID, %d, %s, WEXITED, %s) = 0\n",
		pid, sprint_siginfo(sinfo, "0"), sprint_rusage(rusage));

	long rc = do_waitid(P_ALL, -1, sinfo, WEXITED|WSTOPPED, rusage);
	tprintf("waitid(P_ALL, -1, %p, WEXITED|WSTOPPED, %p)"
		" = %ld %s (%m)\n", sinfo, rusage, rc, errno2name());

	tprintf("%s\n", "+++ exited with 0 +++");
	return 0;
}
