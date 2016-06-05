/*
 * Check decoding of ptrace syscall.
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
#include <sys/syscall.h>

#ifdef __NR_rt_sigprocmask

# include <errno.h>
# include <signal.h>
# include <stdio.h>
# include <string.h>
# include <sys/wait.h>
# include <unistd.h>
# include "ptrace.h"
# include <linux/audit.h>

static long
do_ptrace(unsigned long request, unsigned long pid,
	  unsigned long addr, unsigned long data)
{
	return syscall(__NR_ptrace, request, pid, addr, data);
}

static void
test_peeksiginfo(unsigned long pid, const unsigned long bad_request)
{
	long rc = do_ptrace(PTRACE_PEEKSIGINFO, pid, 0, bad_request);
	printf("ptrace(PTRACE_PEEKSIGINFO, %u, NULL, %#lx)"
	       " = %ld %s (%m)\n", (unsigned) pid, bad_request, rc, errno2name());

	struct {
		unsigned long long off;
		unsigned int flags, nr;
	} *const psi = tail_alloc(sizeof(*psi));

	psi->off = 0xdeadbeeffacefeed;
	psi->flags = 1;
	psi->nr = 42;

	rc = do_ptrace(PTRACE_PEEKSIGINFO,
		       pid, (unsigned long) psi, bad_request);
	printf("ptrace(PTRACE_PEEKSIGINFO, %u, {off=%llu"
	       ", flags=PTRACE_PEEKSIGINFO_SHARED, nr=%u}, %#lx)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, psi->off, psi->nr, bad_request, rc, errno2name());

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
			kill (pid, SIGKILL);
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

		rc = do_ptrace(PTRACE_PEEKSIGINFO, pid,
			       (unsigned long) psi, (unsigned long) sigs);
		if (rc < 0) {
			printf("ptrace(PTRACE_PEEKSIGINFO, %u, {off=%llu"
			       ", flags=0, nr=%u}, %p) = %ld %s (%m)\n",
			       (unsigned) pid, psi->off, psi->nr, sigs,
			       rc, errno2name());
		} else {
			printf("ptrace(PTRACE_PEEKSIGINFO, %u, {off=%llu"
			       ", flags=0, nr=%u}"
			       ", [{si_signo=SIGUSR1, si_code=SI_TKILL"
			       ", si_pid=%u, si_uid=%u}"
			       ", {si_signo=SIGUSR2, si_code=SI_TKILL"
			       ", si_pid=%u, si_uid=%u}"
			       ", {si_signo=SIGALRM, si_code=SI_TKILL"
			       ", si_pid=%u, si_uid=%u}"
			       "]) = %ld\n",
			       (unsigned) pid, psi->off, psi->nr,
			       (unsigned) pid, (unsigned) uid,
			       (unsigned) pid, (unsigned) uid,
			       (unsigned) pid, (unsigned) uid,
			       rc);
		}

		if (do_ptrace(PTRACE_CONT, pid, 0, 0)) {
			saved = errno;
			kill (pid, SIGKILL);
			errno = saved;
			perror_msg_and_fail("ptrace");
		}
		printf("ptrace(PTRACE_CONT, %ld, NULL, SIG_0) = 0\n", pid);
	}
}

int
main(void)
{
	const unsigned long bad_request =
		(unsigned long) 0xdeadbeeffffffeed;
	const unsigned long bad_data =
		(unsigned long) 0xdeadcafefffff00d;
	const unsigned long pid =
		(unsigned long) 0xdefaced00000000 | (unsigned) getpid();

	unsigned int sigset_size;

	for (sigset_size = 1024 / 8; sigset_size; sigset_size >>= 1) {
		if (!syscall(__NR_rt_sigprocmask,
			     SIG_SETMASK, NULL, NULL, sigset_size))
			break;
	}
	if (!sigset_size)
		perror_msg_and_fail("rt_sigprocmask");

	void *const k_set = tail_alloc(sigset_size);
	siginfo_t *const sip = tail_alloc(sizeof(*sip));

	long rc = do_ptrace(bad_request, pid, 0, 0);
	printf("ptrace(%#lx /* PTRACE_??? */, %u, NULL, NULL) = %ld %s (%m)\n",
	       bad_request, (unsigned) pid, rc, errno2name());

	rc = do_ptrace(PTRACE_PEEKDATA, pid, bad_request, bad_data);
# ifdef IA64
	printf("ptrace(PTRACE_PEEKDATA, %u, %#lx)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, rc, errno2name());
# else
	printf("ptrace(PTRACE_PEEKDATA, %u, %#lx, %#lx)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, bad_data, rc, errno2name());
#endif

	rc = do_ptrace(PTRACE_PEEKTEXT, pid, bad_request, bad_data);
# ifdef IA64
	printf("ptrace(PTRACE_PEEKTEXT, %u, %#lx)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, rc, errno2name());
# else
	printf("ptrace(PTRACE_PEEKTEXT, %u, %#lx, %#lx)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, bad_data, rc, errno2name());
#endif

	rc = do_ptrace(PTRACE_PEEKUSER, pid, bad_request, bad_data);
# ifdef IA64
	printf("ptrace(PTRACE_PEEKUSER, %u, %#lx)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, rc, errno2name());
# else
	printf("ptrace(PTRACE_PEEKUSER, %u, %#lx, %#lx)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, bad_data, rc, errno2name());
#endif

	rc = do_ptrace(PTRACE_POKEUSER, pid, bad_request, bad_data);
	printf("ptrace(PTRACE_POKEUSER, %u, %#lx, %#lx)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, bad_data, rc, errno2name());

	rc = do_ptrace(PTRACE_ATTACH, pid, 0, 0);
	printf("ptrace(PTRACE_ATTACH, %u) = %ld %s (%m)\n",
	       (unsigned) pid, rc, errno2name());

	rc = do_ptrace(PTRACE_INTERRUPT, pid, 0, 0);
	printf("ptrace(PTRACE_INTERRUPT, %u) = %ld %s (%m)\n",
	       (unsigned) pid, rc, errno2name());

	rc = do_ptrace(PTRACE_KILL, pid, 0, 0);
	printf("ptrace(PTRACE_KILL, %u) = %ld %s (%m)\n",
	       (unsigned) pid, rc, errno2name());

	rc = do_ptrace(PTRACE_LISTEN, pid, 0, 0);
	printf("ptrace(PTRACE_LISTEN, %u) = %ld %s (%m)\n",
	       (unsigned) pid, rc, errno2name());

	sigset_t libc_set;
	sigemptyset(&libc_set);
	sigaddset(&libc_set, SIGUSR1);
	memcpy(k_set, &libc_set, sigset_size);

	rc = do_ptrace(PTRACE_SETSIGMASK,
		       pid, sigset_size, (unsigned long) k_set);
	printf("ptrace(PTRACE_SETSIGMASK, %u, %u, [USR1])"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, sigset_size, rc, errno2name());

	rc = do_ptrace(PTRACE_GETSIGMASK,
		       pid, sigset_size, (unsigned long) k_set);
	printf("ptrace(PTRACE_GETSIGMASK, %u, %u, %p)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, sigset_size, k_set, rc, errno2name());

	rc = do_ptrace(PTRACE_SECCOMP_GET_FILTER, pid, 42, 0);
	printf("ptrace(PTRACE_SECCOMP_GET_FILTER, %u, 42, NULL)"
	       " = %ld %s (%m)\n", (unsigned) pid, rc, errno2name());

	rc = do_ptrace(PTRACE_GETEVENTMSG, pid, bad_request, bad_data);
	printf("ptrace(PTRACE_GETEVENTMSG, %u, %#lx, %#lx)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, bad_data, rc, errno2name());

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGIO;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_band = -2;

	rc = do_ptrace(PTRACE_SETSIGINFO,
		       pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGIO"
	       ", si_code=POLL_IN, si_errno=ENOENT, si_band=-2})"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, rc, errno2name());

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGTRAP;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_pid = 2;
	sip->si_uid = 3;
	sip->si_ptr = (void *) bad_request;

	rc = do_ptrace(PTRACE_SETSIGINFO,
		       pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGTRAP"
	       ", si_code=TRAP_BRKPT, si_errno=ENOENT, si_pid=2, si_uid=3"
	       ", si_value={int=%d, ptr=%p}}) = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, sip->si_int, sip->si_ptr, rc, errno2name());

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGILL;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_addr = (void *) (unsigned long) 0xfacefeeddeadbeef;

	rc = do_ptrace(PTRACE_SETSIGINFO,
		       pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGILL"
	       ", si_code=ILL_ILLOPC, si_errno=ENOENT, si_addr=%p})"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, sip->si_addr, rc, errno2name());

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGFPE;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_addr = (void *) (unsigned long) 0xfacefeeddeadbeef;

	rc = do_ptrace(PTRACE_SETSIGINFO,
		       pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGFPE"
	       ", si_code=FPE_INTDIV, si_errno=ENOENT, si_addr=%p})"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, sip->si_addr, rc, errno2name());

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGBUS;
	sip->si_code = 1;
	sip->si_errno = -2;
	sip->si_addr = (void *) (unsigned long) 0xfacefeeddeadbeef;

	rc = do_ptrace(PTRACE_SETSIGINFO,
		       pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGBUS"
	       ", si_code=BUS_ADRALN, si_errno=%d, si_addr=%p})"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, sip->si_errno, sip->si_addr,
	       rc, errno2name());

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGPROF;
	sip->si_code = 0xbadc0ded;
	sip->si_errno = -2;
	sip->si_pid = 0;
	sip->si_uid = 3;
	sip->si_ptr = 0;

	rc = do_ptrace(PTRACE_SETSIGINFO,
		       pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGPROF"
	       ", si_code=%#x, si_errno=%d, si_pid=0, si_uid=3})"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, sip->si_code, sip->si_errno,
	       rc, errno2name());

#ifdef HAVE_SIGINFO_T_SI_SYSCALL
	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGSYS;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_call_addr = (void *) (unsigned long) 0xfacefeeddeadbeef;
	sip->si_syscall = -1U;
	sip->si_arch = AUDIT_ARCH_X86_64;

	rc = do_ptrace(PTRACE_SETSIGINFO,
		       pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGSYS"
	       ", si_code=SYS_SECCOMP, si_errno=ENOENT, si_call_addr=%p"
	       ", si_syscall=__NR_syscall_%u, si_arch=AUDIT_ARCH_X86_64})"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, sip->si_call_addr, sip->si_syscall,
	       rc, errno2name());
#endif

#if defined HAVE_SIGINFO_T_SI_TIMERID && defined HAVE_SIGINFO_T_SI_OVERRUN
	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGHUP;
	sip->si_code = SI_TIMER;
	sip->si_errno = ENOENT;
	sip->si_timerid = 0xdeadbeef;
	sip->si_overrun = -1;
	sip->si_ptr = (void *) (unsigned long) 0xfacefeeddeadbeef;

	rc = do_ptrace(PTRACE_SETSIGINFO,
		       pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %u, %#lx, {si_signo=SIGHUP"
	       ", si_code=SI_TIMER, si_errno=ENOENT, si_timerid=%#x"
	       ", si_overrun=%d, si_value={int=%d, ptr=%p}}) = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, sip->si_timerid, sip->si_overrun,
	       sip->si_int, sip->si_ptr, rc, errno2name());
#endif

	rc = do_ptrace(PTRACE_GETSIGINFO,
		       pid, bad_request, (unsigned long) sip);
	printf("ptrace(PTRACE_GETSIGINFO, %u, %#lx, %p)"
	       " = %ld %s (%m)\n", (unsigned) pid, bad_request, sip, rc, errno2name());

	rc = do_ptrace(PTRACE_CONT, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_CONT, %u, NULL, SIGUSR1) = %ld %s (%m)\n",
	       (unsigned) pid, rc, errno2name());

	rc = do_ptrace(PTRACE_DETACH, pid, 0, SIGUSR2);
	printf("ptrace(PTRACE_DETACH, %u, NULL, SIGUSR2) = %ld %s (%m)\n",
	       (unsigned) pid, rc, errno2name());

	rc = do_ptrace(PTRACE_SYSCALL, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_SYSCALL, %u, NULL, SIGUSR1) = %ld %s (%m)\n",
	       (unsigned) pid, rc, errno2name());

#ifdef PTRACE_SINGLESTEP
	rc = do_ptrace(PTRACE_SINGLESTEP, pid, 0, SIGUSR2);
	printf("ptrace(PTRACE_SINGLESTEP, %u, NULL, SIGUSR2) = %ld %s (%m)\n",
	       (unsigned) pid, rc, errno2name());
#endif

#ifdef PTRACE_SINGLEBLOCK
	rc = do_ptrace(PTRACE_SINGLEBLOCK, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_SINGLEBLOCK, %u, NULL, SIGUSR1) = %ld %s (%m)\n",
	       (unsigned) pid, rc, errno2name());
#endif

#ifdef PTRACE_SYSEMU
	rc = do_ptrace(PTRACE_SYSEMU, pid, 0, SIGUSR2);
	printf("ptrace(PTRACE_SYSEMU, %u, NULL, SIGUSR2) = %ld %s (%m)\n",
	       (unsigned) pid, rc, errno2name());
#endif
#ifdef PTRACE_SYSEMU_SINGLESTEP
	rc = do_ptrace(PTRACE_SYSEMU_SINGLESTEP, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_SYSEMU_SINGLESTEP, %u, NULL, SIGUSR1)"
	       " = %ld %s (%m)\n", (unsigned) pid, rc, errno2name());
#endif

	rc = do_ptrace(PTRACE_SETOPTIONS,
		       pid, 0, PTRACE_O_TRACEFORK|PTRACE_O_TRACECLONE);
	printf("ptrace(PTRACE_SETOPTIONS, %u, NULL"
	       ", PTRACE_O_TRACEFORK|PTRACE_O_TRACECLONE) = %ld %s (%m)\n",
	       (unsigned) pid, rc, errno2name());

	rc = do_ptrace(PTRACE_SEIZE,
		       pid, bad_request, PTRACE_O_TRACESYSGOOD);
	printf("ptrace(PTRACE_SEIZE, %u, %#lx"
	       ", PTRACE_O_TRACESYSGOOD) = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, rc, errno2name());

	rc = do_ptrace(PTRACE_SETREGSET, pid, 1, bad_request);
	printf("ptrace(PTRACE_SETREGSET, %u, NT_PRSTATUS, %#lx)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, rc, errno2name());

	rc = do_ptrace(PTRACE_GETREGSET, pid, 3, bad_request);
	printf("ptrace(PTRACE_GETREGSET, %u, NT_PRPSINFO, %#lx)"
	       " = %ld %s (%m)\n",
	       (unsigned) pid, bad_request, rc, errno2name());

	test_peeksiginfo(pid, bad_request);

	rc = do_ptrace(PTRACE_TRACEME, 0, 0, 0);
	if (rc)
		printf("ptrace(PTRACE_TRACEME) = %ld %s (%m)\n",
		       rc, errno2name());
	else
		printf("ptrace(PTRACE_TRACEME) = 0\n");

	puts("+++ exited with 0 +++");
	return 0;
}


#else

SKIP_MAIN_UNDEFINED("__NR_rt_sigprocmask")

#endif
