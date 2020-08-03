/*
 * Check decoding of ptrace syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <errno.h>
#include "ptrace.h"
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/audit.h>

static const char *errstr;

static long
do_ptrace(unsigned long request, unsigned long pid,
	  unsigned long addr, unsigned long data)
{
	long rc = syscall(__NR_ptrace, request, pid, addr, data);
	errstr = sprintrc(rc);
	return rc;
}

static void
test_peeksiginfo(unsigned long pid, const unsigned long bad_request)
{
	do_ptrace(PTRACE_PEEKSIGINFO, pid, 0, bad_request);
	printf("ptrace(PTRACE_PEEKSIGINFO, %u, NULL, %#lx) = %s\n",
	       (unsigned) pid, bad_request, errstr);

	struct psi {
		unsigned long long off;
		unsigned int flags, nr;
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(struct psi, psi);

	psi->off = 0xdeadbeeffacefeedULL;
	psi->flags = 1;
	psi->nr = 42;

	do_ptrace(PTRACE_PEEKSIGINFO, pid, (unsigned long) psi, bad_request);
	printf("ptrace(PTRACE_PEEKSIGINFO, %u, {off=%llu"
	       ", flags=PTRACE_PEEKSIGINFO_SHARED, nr=%u}, %#lx) = %s\n",
	       (unsigned) pid, psi->off, psi->nr, bad_request, errstr);

	pid = fork();
	if ((pid_t) pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGUSR1);
		sigaddset(&mask, SIGUSR2);
		sigaddset(&mask, SIGALRM);

		if (sigprocmask(SIG_BLOCK, &mask, NULL))
			perror_msg_and_fail("sigprocmask");

		raise(SIGUSR1);
		raise(SIGUSR2);
		raise(SIGALRM);

		if (do_ptrace(PTRACE_TRACEME, 0, 0, 0))
			perror_msg_and_fail("child: PTRACE_TRACEME");

		raise(SIGSTOP);
		_exit(0);
	}

	const unsigned int nsigs = 4;
	const uid_t uid = geteuid();
	siginfo_t *sigs = tail_alloc(sizeof(*sigs) * nsigs);

	psi->off = 0;
	psi->flags = 0;
	psi->nr = nsigs;

	for (;;) {
		int status, tracee, saved;

		errno = 0;
		tracee = wait(&status);
		if (tracee <= 0) {
			if (errno == EINTR)
				continue;
			saved = errno;
			kill(pid, SIGKILL);
			errno = saved;
			perror_msg_and_fail("wait");
		}
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0)
				break;
			error_msg_and_fail("unexpected exit status %u",
					   WEXITSTATUS(status));
		}
		if (WIFSIGNALED(status))
			error_msg_and_fail("unexpected signal %u",
					   WTERMSIG(status));
		if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP) {
			kill(pid, SIGKILL);
			error_msg_and_fail("unexpected wait status %x",
					   status);
		}

		long rc = do_ptrace(PTRACE_PEEKSIGINFO, pid,
				    (unsigned long) psi, (unsigned long) sigs);
		if (rc < 0) {
			printf("ptrace(PTRACE_PEEKSIGINFO, %u"
			       ", {off=%llu, flags=0, nr=%u}, %p) = %s\n",
			       (unsigned) pid, psi->off, psi->nr, sigs,
			       errstr);
		} else {
			printf("ptrace(PTRACE_PEEKSIGINFO, %u"
			       ", {off=%llu, flags=0, nr=%u}"
			       ", [{si_signo=SIGUSR1, si_code=SI_TKILL"
			       ", si_pid=%d, si_uid=%d}"
			       ", {si_signo=SIGUSR2, si_code=SI_TKILL"
			       ", si_pid=%d, si_uid=%d}"
			       ", {si_signo=SIGALRM, si_code=SI_TKILL"
			       ", si_pid=%d, si_uid=%d}"
			       "]) = %s\n",
			       (unsigned) pid, psi->off, psi->nr,
			       (int) pid, (int) uid,
			       (int) pid, (int) uid,
			       (int) pid, (int) uid,
			       errstr);
		}

		if (do_ptrace(PTRACE_CONT, pid, 0, 0)) {
			saved = errno;
			kill(pid, SIGKILL);
			errno = saved;
			perror_msg_and_fail("ptrace");
		}
		printf("ptrace(PTRACE_CONT, %ld, NULL, 0) = 0\n", pid);
	}
}

int
main(void)
{
	const unsigned long bad_request =
		(unsigned long) 0xdeadbeeffffffeedULL;
	const unsigned long bad_data =
		(unsigned long) 0xdeadcafefffff00dULL;
	const unsigned long pid =
		(unsigned long) 0xdefaced00000000ULL | (unsigned) getpid();

	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, filter_off);

	const unsigned int sigset_size = get_sigset_size();

	void *const k_set = tail_alloc(sigset_size);
	TAIL_ALLOC_OBJECT_CONST_PTR(siginfo_t, sip);

	do_ptrace(bad_request, pid, 0, 0);
	printf("ptrace(%#lx /* PTRACE_??? */, %u, NULL, NULL) = %s\n",
	       bad_request, (unsigned) pid, errstr);

	do_ptrace(PTRACE_PEEKDATA, pid, bad_request, bad_data);
#ifdef IA64
	printf("ptrace(PTRACE_PEEKDATA, %u, %#lx) = %s\n",
	       (unsigned) pid, bad_request, errstr);
#else
	printf("ptrace(PTRACE_PEEKDATA, %u, %#lx, %#lx) = %s\n",
	       (unsigned) pid, bad_request, bad_data, errstr);
#endif

	do_ptrace(PTRACE_PEEKTEXT, pid, bad_request, bad_data);
#ifdef IA64
	printf("ptrace(PTRACE_PEEKTEXT, %u, %#lx) = %s\n",
	       (unsigned) pid, bad_request, errstr);
#else
	printf("ptrace(PTRACE_PEEKTEXT, %u, %#lx, %#lx) = %s\n",
	       (unsigned) pid, bad_request, bad_data, errstr);
#endif

	do_ptrace(PTRACE_PEEKUSER, pid, bad_request, bad_data);
#ifdef IA64
	printf("ptrace(PTRACE_PEEKUSER, %u, %#lx) = %s\n",
	       (unsigned) pid, bad_request, errstr);
#else
	printf("ptrace(PTRACE_PEEKUSER, %u, %#lx, %#lx) = %s\n",
	       (unsigned) pid, bad_request, bad_data, errstr);
#endif

	do_ptrace(PTRACE_POKEUSER, pid, bad_request, bad_data);
	printf("ptrace(PTRACE_POKEUSER, %u, %#lx, %#lx) = %s\n",
	       (unsigned) pid, bad_request, bad_data, errstr);

	do_ptrace(PTRACE_ATTACH, pid, 0, 0);
	printf("ptrace(PTRACE_ATTACH, %u) = %s\n", (unsigned) pid, errstr);

	do_ptrace(PTRACE_INTERRUPT, pid, 0, 0);
	printf("ptrace(PTRACE_INTERRUPT, %u) = %s\n", (unsigned) pid, errstr);

	do_ptrace(PTRACE_KILL, pid, 0, 0);
	printf("ptrace(PTRACE_KILL, %u) = %s\n", (unsigned) pid, errstr);

	do_ptrace(PTRACE_LISTEN, pid, 0, 0);
	printf("ptrace(PTRACE_LISTEN, %u) = %s\n", (unsigned) pid, errstr);

	sigset_t libc_set;
	sigemptyset(&libc_set);
	sigaddset(&libc_set, SIGUSR1);
	memcpy(k_set, &libc_set, sigset_size);

	do_ptrace(PTRACE_SETSIGMASK, pid, sigset_size, (unsigned long) k_set);
	printf("ptrace(PTRACE_SETSIGMASK, %u, %u, [USR1]) = %s\n",
	       (unsigned) pid, sigset_size, errstr);

	do_ptrace(PTRACE_GETSIGMASK, pid, sigset_size, (unsigned long) k_set);
	printf("ptrace(PTRACE_GETSIGMASK, %u, %u, %p) = %s\n",
	       (unsigned) pid, sigset_size, k_set, errstr);

	do_ptrace(PTRACE_SECCOMP_GET_FILTER, pid, 42, 0);
	printf("ptrace(PTRACE_SECCOMP_GET_FILTER, %u, 42, NULL) = %s\n",
	       (unsigned) pid, errstr);

	do_ptrace(PTRACE_SECCOMP_GET_METADATA, pid, bad_data, 0);
	printf("ptrace(PTRACE_SECCOMP_GET_METADATA, %u, %lu, NULL) = %s\n",
	       (unsigned) pid, bad_data, errstr);

	do_ptrace(PTRACE_SECCOMP_GET_METADATA, pid, 7,
		  (unsigned long) filter_off);
	printf("ptrace(PTRACE_SECCOMP_GET_METADATA, %u, 7, %p) = %s\n",
	       (unsigned) pid, filter_off, errstr);

	*filter_off = 0xfacefeeddeadc0deULL;
	do_ptrace(PTRACE_SECCOMP_GET_METADATA, pid, bad_data,
		  (unsigned long) filter_off);
	printf("ptrace(PTRACE_SECCOMP_GET_METADATA, %u, %lu, "
	       "{filter_off=%" PRIu64 "}) = %s\n",
	       (unsigned) pid, bad_data, *filter_off, errstr);

	do_ptrace(PTRACE_GETEVENTMSG, pid, bad_request, bad_data);
	printf("ptrace(PTRACE_GETEVENTMSG, %u, %#lx, %#lx) = %s\n",
	       (unsigned) pid, bad_request, bad_data, errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGIO;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_band = -2;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGIO"
	       ", si_code=POLL_IN, si_errno=ENOENT, si_band=-2}) = %s\n",
	       (unsigned) pid, bad_request, errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGTRAP;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_pid = 2;
	sip->si_uid = 3;
	sip->si_ptr = (void *) bad_request;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGTRAP"
	       ", si_code=TRAP_BRKPT, si_errno=ENOENT, si_pid=2, si_uid=3"
	       ", si_value={int=%d, ptr=%p}}) = %s\n",
	       (unsigned) pid, bad_request, sip->si_int, sip->si_ptr,
	       errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGILL;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_addr = (void *) (unsigned long) 0xfacefeeddeadbeefULL;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGILL"
	       ", si_code=ILL_ILLOPC, si_errno=ENOENT, si_addr=%p}) = %s\n",
	       (unsigned) pid, bad_request, sip->si_addr, errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGFPE;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_addr = (void *) (unsigned long) 0xfacefeeddeadbeefULL;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGFPE"
	       ", si_code=FPE_INTDIV, si_errno=ENOENT, si_addr=%p}) = %s\n",
	       (unsigned) pid, bad_request, sip->si_addr, errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGBUS;
	sip->si_code = 1;
	sip->si_errno = -2;
	sip->si_addr = (void *) (unsigned long) 0xfacefeeddeadbeefULL;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGBUS"
	       ", si_code=BUS_ADRALN, si_errno=%u, si_addr=%p}) = %s\n",
	       (unsigned) pid, bad_request, sip->si_errno, sip->si_addr,
	       errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGPROF;
	sip->si_code = 0xbadc0ded;
	sip->si_errno = -2;
	sip->si_pid = 0;
	sip->si_uid = 3;
	sip->si_ptr = 0;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGPROF"
	       ", si_code=%#x, si_errno=%u, si_pid=0, si_uid=3}) = %s\n",
	       (unsigned) pid, bad_request, sip->si_code, sip->si_errno,
	       errstr);

#ifdef HAVE_SIGINFO_T_SI_SYSCALL
	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGSYS;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_call_addr = (void *) (unsigned long) 0xfacefeeddeadbeefULL;
	sip->si_syscall = -1U;
	sip->si_arch = AUDIT_ARCH_X86_64;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGSYS"
	       ", si_code=SYS_SECCOMP, si_errno=ENOENT, si_call_addr=%p"
	       ", si_syscall=%u, si_arch=AUDIT_ARCH_X86_64})"
	       " = %s\n",
	       (unsigned) pid, bad_request, sip->si_call_addr, sip->si_syscall,
	       errstr);

	sip->si_errno = 3141592653U;
	sip->si_call_addr = NULL;
	sip->si_syscall = __NR_read;
	sip->si_arch = 0xda7a1057;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGSYS"
	       ", si_code=SYS_SECCOMP, si_errno=%u, si_call_addr=NULL"
	       ", si_syscall=__NR_read, si_arch=%#x /* AUDIT_ARCH_??? */})"
	       " = %s\n",
	       (unsigned) pid, bad_request, sip->si_errno, sip->si_arch,
	       errstr);
#endif

#if defined HAVE_SIGINFO_T_SI_TIMERID && defined HAVE_SIGINFO_T_SI_OVERRUN
	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGHUP;
	sip->si_code = SI_TIMER;
	sip->si_errno = ENOENT;
	sip->si_timerid = 0xdeadbeef;
	sip->si_overrun = -1;
	sip->si_ptr = (void *) (unsigned long) 0xfacefeeddeadbeefULL;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGHUP"
	       ", si_code=SI_TIMER, si_errno=ENOENT, si_timerid=%#x"
	       ", si_overrun=%d, si_value={int=%d, ptr=%p}}) = %s\n",
	       (unsigned) pid, bad_request, sip->si_timerid, sip->si_overrun,
	       sip->si_int, sip->si_ptr, errstr);
#endif

	do_ptrace(PTRACE_GETSIGINFO, pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_GETSIGINFO, %u, %#lx, %p)"
	       " = %s\n", (unsigned) pid, bad_request, sip, errstr);

	do_ptrace(PTRACE_CONT, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_CONT, %u, NULL, SIGUSR1) = %s\n",
	       (unsigned) pid, errstr);

	do_ptrace(PTRACE_DETACH, pid, 0, SIGUSR2);
	printf("ptrace(PTRACE_DETACH, %u, NULL, SIGUSR2) = %s\n",
	       (unsigned) pid, errstr);

	do_ptrace(PTRACE_SYSCALL, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_SYSCALL, %u, NULL, SIGUSR1) = %s\n",
	       (unsigned) pid, errstr);

#ifdef PTRACE_SINGLESTEP
	do_ptrace(PTRACE_SINGLESTEP, pid, 0, SIGUSR2);
	printf("ptrace(PTRACE_SINGLESTEP, %u, NULL, SIGUSR2) = %s\n",
	       (unsigned) pid, errstr);
#endif

#ifdef PTRACE_SINGLEBLOCK
	do_ptrace(PTRACE_SINGLEBLOCK, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_SINGLEBLOCK, %u, NULL, SIGUSR1) = %s\n",
	       (unsigned) pid, errstr);
#endif

#ifdef PTRACE_SYSEMU
	do_ptrace(PTRACE_SYSEMU, pid, 0, SIGUSR2);
	printf("ptrace(PTRACE_SYSEMU, %u, NULL, SIGUSR2) = %s\n",
	       (unsigned) pid, errstr);
#endif
#ifdef PTRACE_SYSEMU_SINGLESTEP
	do_ptrace(PTRACE_SYSEMU_SINGLESTEP, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_SYSEMU_SINGLESTEP, %u, NULL, SIGUSR1) = %s\n",
	       (unsigned) pid, errstr);
#endif

	do_ptrace(PTRACE_SETOPTIONS,
		  pid, 0, PTRACE_O_TRACEFORK|PTRACE_O_TRACECLONE);
	printf("ptrace(PTRACE_SETOPTIONS, %u, NULL"
	       ", PTRACE_O_TRACEFORK|PTRACE_O_TRACECLONE) = %s\n",
	       (unsigned) pid, errstr);

	do_ptrace(PTRACE_SEIZE, pid, bad_request, PTRACE_O_TRACESYSGOOD);
	printf("ptrace(PTRACE_SEIZE, %u, %#lx, PTRACE_O_TRACESYSGOOD) = %s\n",
	       (unsigned) pid, bad_request, errstr);

	do_ptrace(PTRACE_SETREGSET, pid, 1, bad_request);
	printf("ptrace(PTRACE_SETREGSET, %u, NT_PRSTATUS, %#lx) = %s\n",
	       (unsigned) pid, bad_request, errstr);

	do_ptrace(PTRACE_GETREGSET, pid, 3, bad_request);
	printf("ptrace(PTRACE_GETREGSET, %u, NT_PRPSINFO, %#lx) = %s\n",
	       (unsigned) pid, bad_request, errstr);

	test_peeksiginfo(pid, bad_request);

	do_ptrace(PTRACE_TRACEME, 0, 0, 0);
	printf("ptrace(PTRACE_TRACEME) = %s\n", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
