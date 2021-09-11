/*
 * Check decoding of ptrace syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "print_fields.h"

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
#include <sys/uio.h>
#include <sys/user.h>

#include "cur_audit_arch.h"

#include "xlat.h"
#define XLAT_MACROS_ONLY
# include "xlat/elf_em.h"
#undef XLAT_MACROS_ONLY
#include "xlat/audit_arch.h"

static const char *errstr;

static long
do_ptrace(const unsigned long request,
	  const unsigned int pid,
	  const unsigned long addr,
	  const unsigned long data)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = request;
	const kernel_ulong_t arg2 = fill | pid;
	const kernel_ulong_t arg3 = addr;
	const kernel_ulong_t arg4 = data;
	long rc = syscall(__NR_ptrace, arg1, arg2, arg3, arg4, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

#if defined PTRACE_GETREGS || defined PTRACE_GETREGS64 || defined PTRACE_GETFPREGS
static long
do_ptrace_regs(const unsigned long request,
	       const unsigned int pid,
	       const unsigned long regs_addr)
{
	const unsigned long bad = (unsigned long) 0xdeadbeefdeadbeefULL;
# ifdef __sparc__
	const unsigned long arg_addr = regs_addr;
	const unsigned long arg_data = bad;
# else
	const unsigned long arg_addr = bad;
	const unsigned long arg_data = regs_addr;
# endif
	return do_ptrace(request, pid, arg_addr, arg_data);
}
#endif /* PTRACE_GETREGS || PTRACE_GETREGS64 || PTRACE_GETFPREGS */

static void
test_peeksiginfo(int pid, const unsigned long bad_request)
{
	do_ptrace(PTRACE_PEEKSIGINFO, pid, 0, bad_request);
	printf("ptrace(PTRACE_PEEKSIGINFO, %d, NULL, %#lx) = %s\n",
	       pid, bad_request, errstr);

	struct psi {
		unsigned long long off;
		unsigned int flags, nr;
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(struct psi, psi);

	psi->off = 0xdeadbeeffacefeedULL;
	psi->flags = 1;
	psi->nr = 42;

	do_ptrace(PTRACE_PEEKSIGINFO, pid, (uintptr_t) psi, bad_request);
	printf("ptrace(PTRACE_PEEKSIGINFO, %d, {off=%llu"
	       ", flags=PTRACE_PEEKSIGINFO_SHARED, nr=%u}, %#lx) = %s\n",
	       pid, psi->off, psi->nr, bad_request, errstr);

	pid = fork();
	if (pid < 0)
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
			error_msg_and_fail("unexpected exit status %#x",
					   WEXITSTATUS(status));
		}
		if (WIFSIGNALED(status))
			error_msg_and_fail("unexpected signal %u",
					   WTERMSIG(status));
		if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP) {
			kill(pid, SIGKILL);
			error_msg_and_fail("unexpected wait status %#x",
					   status);
		}

		long rc = do_ptrace(PTRACE_PEEKSIGINFO, pid,
				    (uintptr_t) psi, (uintptr_t) sigs);
		if (rc < 0) {
			printf("ptrace(PTRACE_PEEKSIGINFO, %d"
			       ", {off=%llu, flags=0, nr=%u}, %p) = %s\n",
			       pid, psi->off, psi->nr, sigs, errstr);
		} else {
			printf("ptrace(PTRACE_PEEKSIGINFO, %d"
			       ", {off=%llu, flags=0, nr=%u}"
			       ", [{si_signo=SIGUSR1, si_code=SI_TKILL"
			       ", si_pid=%d, si_uid=%d}"
			       ", {si_signo=SIGUSR2, si_code=SI_TKILL"
			       ", si_pid=%d, si_uid=%d}"
			       ", {si_signo=SIGALRM, si_code=SI_TKILL"
			       ", si_pid=%d, si_uid=%d}"
			       "]) = %s\n",
			       pid, psi->off, psi->nr,
			       pid, (int) uid,
			       pid, (int) uid,
			       pid, (int) uid,
			       errstr);
		}

		if (do_ptrace(PTRACE_CONT, pid, 0, 0)) {
			saved = errno;
			kill(pid, SIGKILL);
			errno = saved;
			perror_msg_and_fail("ptrace");
		}
		printf("ptrace(PTRACE_CONT, %d, NULL, 0) = 0\n", pid);
	}
}

#undef TRACEE_REGS_STRUCT
#if defined __x86_64__ || defined __i386__
# define TRACEE_REGS_STRUCT struct user_regs_struct
#elif defined __powerpc__ || defined __powerpc64__
# define TRACEE_REGS_STRUCT struct pt_regs
#elif defined __arm__
# define TRACEE_REGS_STRUCT struct pt_regs
#elif defined __arm64__ || defined __aarch64__
# define TRACEE_REGS_STRUCT struct user_pt_regs
#elif defined __s390__ || defined __s390x__
# define TRACEE_REGS_STRUCT s390_regs
#elif defined __sparc__
# ifdef __arch64__
typedef struct {
	unsigned long g[8];
	unsigned long o[8];
	unsigned long l[8];
	unsigned long i[8];
	unsigned long tstate;
	unsigned long tpc;
	unsigned long tnpc;
	unsigned long y;
} sparc64_regs;
#  define TRACEE_REGS_STRUCT sparc64_regs
# else /* sparc32 */
typedef struct {
	unsigned int g[8];
	unsigned int o[8];
	unsigned int l[8];
	unsigned int i[8];
	unsigned int psr;
	unsigned int pc;
	unsigned int npc;
	unsigned int y;
	unsigned int wim;
	unsigned int tbr;
} sparc32_regs;
#  define TRACEE_REGS_STRUCT sparc32_regs
# endif
#elif defined __riscv__
# define TRACEE_REGS_STRUCT struct user_regs_struct
#elif defined __mips__
typedef struct {
# ifdef LINUX_MIPSO32
	unsigned long unused[6];
# endif
	unsigned long regs[32];
	unsigned long lo;
	unsigned long hi;
	unsigned long cp0_epc;
	unsigned long cp0_badvaddr;
	unsigned long cp0_status;
	unsigned long cp0_cause;
} mips_regs;
# define TRACEE_REGS_STRUCT mips_regs
#endif

static void
print_prstatus_regset(const void *const rs, const size_t size)
{
	if (!size || size % sizeof(kernel_ulong_t)) {
		printf("%p", rs);
		return;
	}

#ifdef TRACEE_REGS_STRUCT
	const TRACEE_REGS_STRUCT *const regs = rs;

	fputs("{", stdout);

# if defined __x86_64__

	PRINT_FIELD_X(*regs, r15);
	if (size >= offsetofend(TRACEE_REGS_STRUCT, r14)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, r14);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, r13)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, r13);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, r12)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, r12);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, rbp)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, rbp);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, rbx)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, rbx);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, r11)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, r11);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, r10)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, r10);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, r9)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, r9);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, r8)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, r8);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, rax)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, rax);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, rcx)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, rcx);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, rdx)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, rdx);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, rsi)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, rsi);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, rdi)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, rdi);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, orig_rax)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, orig_rax);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, rip)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, rip);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, cs)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, cs);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, eflags)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, eflags);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, rsp)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, rsp);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, ss)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, ss);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, fs_base)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, fs_base);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, gs_base)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, gs_base);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, ds)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, ds);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, es)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, es);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, fs)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, fs);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, gs)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, gs);
	}

# elif defined __i386__

	PRINT_FIELD_X(*regs, ebx);
	if (size >= offsetofend(TRACEE_REGS_STRUCT, ecx)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, ecx);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, edx)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, edx);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, esi)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, esi);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, edi)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, edi);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, ebp)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, ebp);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, eax)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, eax);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, xds)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, xds);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, xes)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, xes);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, xfs)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, xfs);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, xgs)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, xgs);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, orig_eax)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, orig_eax);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, eip)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, eip);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, xcs)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, xcs);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, eflags)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, eflags);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, esp)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, esp);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, xss)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, xss);
	}

# elif defined __powerpc__ || defined __powerpc64__

	fputs("gpr=[", stdout);
	for (unsigned int i = 0; i < ARRAY_SIZE(regs->gpr); ++i) {
		if (size > i * sizeof(regs->gpr[i])) {
			if (i)
				fputs(", ", stdout);
			PRINT_VAL_X(regs->gpr[i]);
		}
	}
	fputs("]", stdout);
	if (size >= offsetofend(TRACEE_REGS_STRUCT, nip)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, nip);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, msr)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, msr);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, orig_gpr3)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, orig_gpr3);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, ctr)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, ctr);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, link)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, link);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, xer)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, xer);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, ccr)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, ccr);
	}
#  ifdef __powerpc64__
	if (size >= offsetofend(TRACEE_REGS_STRUCT, softe)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, softe);
	}
#  else
	if (size >= offsetofend(TRACEE_REGS_STRUCT, mq)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, mq);
	}
#  endif
	if (size >= offsetofend(TRACEE_REGS_STRUCT, trap)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, trap);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, dar)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, dar);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, dsisr)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, dsisr);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, result)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, result);
	}

# elif defined __arm__

	fputs("uregs=[", stdout);
	for (unsigned int i = 0; i < ARRAY_SIZE(regs->uregs); ++i) {
		if (size > i * sizeof(regs->uregs[i])) {
			if (i)
				fputs(", ", stdout);
			PRINT_VAL_X(regs->uregs[i]);
		}
	}
	fputs("]", stdout);

# elif defined __arm64__ || defined __aarch64__

	fputs("regs=[", stdout);
	for (unsigned int i = 0; i < ARRAY_SIZE(regs->regs); ++i) {
		if (size > i * sizeof(regs->regs[i])) {
			if (i)
				fputs(", ", stdout);
			PRINT_VAL_X(regs->regs[i]);
		}
	}
	fputs("]", stdout);
	if (size >= offsetofend(TRACEE_REGS_STRUCT, sp)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, sp);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, pc)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, pc);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, pstate)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, pstate);
	}

# elif defined __s390__ || defined __s390x__

	fputs("psw={", stdout);
	PRINT_FIELD_X(regs->psw, mask);
	if (size >= sizeof(regs->psw)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(regs->psw, addr);
	}
	fputs("}", stdout);
	if (size > offsetof(TRACEE_REGS_STRUCT, gprs)) {
		const size_t len = size - offsetof(TRACEE_REGS_STRUCT, gprs);
		fputs(", gprs=[", stdout);
		for (unsigned int i = 0; i < ARRAY_SIZE(regs->gprs); ++i) {
			if (len > i * sizeof(regs->gprs[i])) {
				if (i)
					fputs(", ", stdout);
				PRINT_VAL_X(regs->gprs[i]);
			}
		}
		fputs("]", stdout);
	}
	if (size > offsetof(TRACEE_REGS_STRUCT, acrs)) {
		const size_t len = size - offsetof(TRACEE_REGS_STRUCT, acrs);
		fputs(", acrs=[", stdout);
		for (unsigned int i = 0; i < ARRAY_SIZE(regs->acrs); ++i) {
			if (len > i * sizeof(regs->acrs[i])) {
				if (i)
					fputs(", ", stdout);
				PRINT_VAL_X(regs->acrs[i]);
			}
		}
		fputs("]", stdout);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, orig_gpr2)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, orig_gpr2);
	}

# elif defined __sparc__

	fputs("g=[", stdout);
	for (unsigned int j = 0; j < ARRAY_SIZE(regs->g); ++j) {
		if (size > j * sizeof(regs->g[j])) {
			if (j)
				fputs(", ", stdout);
			PRINT_VAL_X(regs->g[j]);
		}
	}
	fputs("]", stdout);
	if (size > offsetof(TRACEE_REGS_STRUCT, o)) {
		const size_t len = size - offsetof(TRACEE_REGS_STRUCT, o);
		fputs(", o=[", stdout);
		for (unsigned int j = 0; j < ARRAY_SIZE(regs->o); ++j) {
			if (len > j * sizeof(regs->o[j])) {
				if (j)
					fputs(", ", stdout);
				PRINT_VAL_X(regs->o[j]);
			}
		}
		fputs("]", stdout);
	}
	if (size > offsetof(TRACEE_REGS_STRUCT, l)) {
		const size_t len = size - offsetof(TRACEE_REGS_STRUCT, l);
		fputs(", l=[", stdout);
		for (unsigned int j = 0; j < ARRAY_SIZE(regs->l); ++j) {
			if (len > j * sizeof(regs->l[j])) {
				if (j)
					fputs(", ", stdout);
				PRINT_VAL_X(regs->l[j]);
			}
		}
		fputs("]", stdout);
	}
	if (size > offsetof(TRACEE_REGS_STRUCT, i)) {
		const size_t len = size - offsetof(TRACEE_REGS_STRUCT, i);
		fputs(", i=[", stdout);
		for (unsigned int j = 0; j < ARRAY_SIZE(regs->i); ++j) {
			if (len > j * sizeof(regs->i[j])) {
				if (j)
					fputs(", ", stdout);
				PRINT_VAL_X(regs->i[j]);
			}
		}
		fputs("]", stdout);
	}
#  ifdef __arch64__
	if (size >= offsetofend(TRACEE_REGS_STRUCT, tstate)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, tstate);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, tpc)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, tpc);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, tnpc)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, tnpc);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, y)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, y);
	}
#  else /* sparc32 */
	if (size >= offsetofend(TRACEE_REGS_STRUCT, psr)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, psr);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, pc)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, pc);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, npc)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, npc);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, y)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, y);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, wim)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, wim);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, tbr)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, tbr);
	}
#  endif

# elif defined __riscv__

	if (size >= offsetofend(TRACEE_REGS_STRUCT, pc)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, pc);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, ra)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, ra);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, sp)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, sp);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, gp)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, gp);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, tp)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, tp);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, t0)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, t0);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, t1)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, t1);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, t2)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, t2);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s0)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s0);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s1)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s1);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, a0)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, a0);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, a1)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, a1);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, a2)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, a2);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, a3)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, a3);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, a4)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, a4);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, a5)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, a5);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, a6)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, a6);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, a7)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, a7);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s2)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s2);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s3)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s3);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s4)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s4);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s5)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s5);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s6)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s6);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s7)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s7);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s8)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s8);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s9)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s9);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s10)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s10);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, s11)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, s11);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, t3)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, t3);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, t4)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, t4);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, t5)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, t5);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, t6)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, t6);
	}

# elif defined __mips__

	if (size > offsetof(TRACEE_REGS_STRUCT, regs)) {
		const size_t len = size - offsetof(TRACEE_REGS_STRUCT, regs);
		fputs("regs=[", stdout);
		for (unsigned int i = 0; i < ARRAY_SIZE(regs->regs); ++i) {
			if (len > i * sizeof(regs->regs[i])) {
				if (i)
					fputs(", ", stdout);
				PRINT_VAL_X(regs->regs[i]);
			}
		}
		fputs("]", stdout);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, lo)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, lo);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, hi)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, hi);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, cp0_epc)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, cp0_epc);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, cp0_badvaddr)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, cp0_badvaddr);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, cp0_status)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, cp0_status);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, cp0_cause)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, cp0_cause);
	}

# endif /*
	   __aarch64__ ||
	   __arm64__ ||
	   __arm__ ||
	   __i386__ ||
	   __mips__ ||
	   __powerpc64__ ||
	   __powerpc__ ||
	   __riscv__ ||
	   __s390__ ||
	   __s390x__ ||
	   __sparc__ ||
	   __x86_64__
	 */

	if (size > sizeof(*regs))
		fputs(", ...", stdout);
	fputs("}", stdout);

#else /* !TRACEE_REGS_STRUCT */

	printf("%p", rs);

#endif /* TRACEE_REGS_STRUCT */
}

#ifdef PTRACE_GETREGS
static void
print_pt_regs(const void *const rs, const size_t size)
{
# if defined __mips__

	const struct pt_regs *const regs = rs;
	if (size != sizeof(*regs))
		error_msg_and_fail("expected size %zu, got size %zu",
				   sizeof(*regs), size);
	fputs("{regs=[", stdout);
	for (unsigned int j = 0; j < ARRAY_SIZE(regs->regs); ++j) {
		if (j)
			fputs(", ", stdout);
		printf("%#llx", (unsigned long long) regs->regs[j]);
	}
	fputs("], ", stdout);
	PRINT_FIELD_X(*regs, lo);
	fputs(", ", stdout);
	PRINT_FIELD_X(*regs, hi);
	fputs(", ", stdout);
	PRINT_FIELD_X(*regs, cp0_epc);
	fputs(", ", stdout);
	PRINT_FIELD_X(*regs, cp0_badvaddr);
	fputs(", ", stdout);
	PRINT_FIELD_X(*regs, cp0_status);
	fputs(", ", stdout);
	PRINT_FIELD_X(*regs, cp0_cause);
	fputs("}", stdout);

# elif defined __sparc__

#  ifdef __arch64__
	printf("%p", rs);
#  else
	const struct {
		unsigned int psr;
		unsigned int pc;
		unsigned int npc;
		unsigned int y;
		unsigned int u_regs[15];
	} *const regs = rs;
	if (size != sizeof(*regs))
		error_msg_and_fail("expected size %zu, got size %zu",
				   sizeof(*regs), size);
	printf("{psr=%#x, pc=%#x, npc=%#x, y=%#x, u_regs=[",
	       regs->psr, regs->pc, regs->npc, regs->y);
	for (unsigned int j = 0; j < ARRAY_SIZE(regs->u_regs); ++j) {
		if (j)
			fputs(", ", stdout);
		printf("%#x", regs->u_regs[j]);
	}
	fputs("]}", stdout);
#  endif /* !__arch64__ */

# elif defined TRACEE_REGS_STRUCT

	if (size != sizeof(TRACEE_REGS_STRUCT))
		error_msg_and_fail("expected size %zu, got size %zu",
				   sizeof(TRACEE_REGS_STRUCT), size);
	print_prstatus_regset(rs, size);

# else /* !TRACEE_REGS_STRUCT */

	printf("%p", rs);

# endif
}

static void
do_getregs_setregs(const int pid,
		   void *const regbuf,
		   const unsigned int regsize,
		   unsigned int *const actual_size)
{
	if (do_ptrace_regs(PTRACE_GETREGS, pid, (uintptr_t) regbuf)) {
		printf("ptrace(PTRACE_GETREGS, %d, %p) = %s\n",
		       pid, regbuf, errstr);
		return;	/* skip PTRACE_SETREGS */
	} else {
		printf("ptrace(PTRACE_GETREGS, %d, ", pid);
		print_pt_regs(regbuf, regsize);
		printf(") = %s\n", errstr);
		if (*actual_size)
			return;	/* skip PTRACE_SETREGS */
		else
			*actual_size = regsize;
	}

# if defined __sparc__ && !defined __arch64__
	/*
	 * On sparc32 PTRACE_SETREGS of size greater than 120
	 * has interesting side effects.
	 */
	if (regsize > 120)
		return;
# endif /* __sparc__ && !__arch64__ */

	do_ptrace_regs(PTRACE_SETREGS, pid, (uintptr_t) regbuf);
	printf("ptrace(PTRACE_SETREGS, %d, ", pid);
	print_pt_regs(regbuf, regsize);
	printf(") = %s\n", errstr);
}
#endif /* PTRACE_GETREGS */

#ifdef PTRACE_GETREGS64
static void
print_pt_regs64(const void *const rs, const size_t size)
{
# if defined __powerpc__ || defined __powerpc64__
#  ifdef __powerpc64__
	const struct pt_regs *const regs = rs;
#  else /* __powerpc__ */
	const struct {
		unsigned long long gpr[32];
		unsigned long long nip;
		unsigned long long msr;
		unsigned long long orig_gpr3;
		unsigned long long ctr;
		unsigned long long link;
		unsigned long long xer;
		unsigned long long ccr;
		unsigned long long softe;
		unsigned long long trap;
		unsigned long long dar;
		unsigned long long dsisr;
		unsigned long long result;
	} *const regs = rs;
#  endif /* __powerpc__ */
	if (size != sizeof(*regs))
		error_msg_and_fail("expected size %zu, got size %zu",
				   sizeof(*regs), size);
	for (unsigned int j = 0; j < ARRAY_SIZE(regs->gpr); ++j)
		printf("%s%#llx", j ? ", " : "{gpr=[",
		       (unsigned long long) regs->gpr[j]);
	printf("], nip=%#llx", (unsigned long long) regs->nip);
	printf(", msr=%#llx", (unsigned long long) regs->msr);
	printf(", orig_gpr3=%#llx", (unsigned long long) regs->orig_gpr3);
	printf(", ctr=%#llx", (unsigned long long) regs->ctr);
	printf(", link=%#llx", (unsigned long long) regs->link);
	printf(", xer=%#llx", (unsigned long long) regs->xer);
	printf(", ccr=%#llx", (unsigned long long) regs->ccr);
	printf(", softe=%#llx", (unsigned long long) regs->softe);
	printf(", trap=%#llx", (unsigned long long) regs->trap);
	printf(", dar=%#llx", (unsigned long long) regs->dar);
	printf(", dsisr=%#llx", (unsigned long long) regs->dsisr);
	printf(", result=%#llx}", (unsigned long long) regs->result);

# elif defined __sparc__ && defined __arch64__

	const struct {
		unsigned long u_regs[16];
		unsigned long tstate;
		unsigned long tpc;
		unsigned long tnpc;
	} *const regs = rs;
	if (size != sizeof(*regs))
		error_msg_and_fail("expected size %zu, got size %zu",
				   sizeof(*regs), size);
	for (unsigned int j = 0; j < ARRAY_SIZE(regs->u_regs); ++j)
		printf("%s%#lx", j ? ", " : "{u_regs=[", regs->u_regs[j]);
	printf("], tstate=%#lx", regs->tstate);
	printf(", tpc=%#lx", regs->tpc);
	printf(", tnpc=%#lx}", regs->tnpc);

# else /* !(__powerpc__ || __powerpc64__ || (__sparc__ && __arch64__)) */

	printf("%p", rs);

# endif
}

static void
do_getregs64_setregs64(const int pid,
		       void *const regbuf,
		       const unsigned int regsize,
		       unsigned int *const actual_size)
{
	if (do_ptrace_regs(PTRACE_GETREGS64, pid, (uintptr_t) regbuf)) {
		printf("ptrace(PTRACE_GETREGS64, %d, %p) = %s\n",
		       pid, regbuf, errstr);
		return;	/* skip PTRACE_SETREGS64 */
	} else {
		printf("ptrace(PTRACE_GETREGS64, %d, ", pid);
		print_pt_regs64(regbuf, regsize);
		printf(") = %s\n", errstr);
		if (*actual_size)
			return;	/* skip PTRACE_SETREGS64 */
		else
			*actual_size = regsize;
	}

	do_ptrace_regs(PTRACE_SETREGS64, pid, (uintptr_t) regbuf);
	printf("ptrace(PTRACE_SETREGS64, %d, ", pid);
	print_pt_regs64(regbuf, regsize);
	printf(") = %s\n", errstr);
}
#endif /* PTRACE_GETREGS64 */

#if defined __powerpc__ || defined __powerpc64__
# define FPREGSET_SLOT_SIZE sizeof(uint64_t)
#else
# define FPREGSET_SLOT_SIZE sizeof(kernel_ulong_t)
#endif

#undef TRACEE_REGS_STRUCT
#if defined __x86_64__ || defined __i386__
# define TRACEE_REGS_STRUCT struct user_fpregs_struct
#elif defined __powerpc__ || defined __powerpc64__
typedef struct {
	uint64_t fpr[32];
	uint64_t fpscr;
} ppc_fpregs_struct;
# define TRACEE_REGS_STRUCT ppc_fpregs_struct
#endif

static void
print_fpregset(const void *const rs, const size_t size)
{
	if (!size || size % FPREGSET_SLOT_SIZE) {
		printf("%p", rs);
		return;
	}

#ifdef TRACEE_REGS_STRUCT
	const TRACEE_REGS_STRUCT *const regs = rs;

	fputs("{", stdout);

# if defined __i386__

	PRINT_FIELD_X(*regs, cwd);
	if (size >= offsetofend(TRACEE_REGS_STRUCT, swd)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, swd);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, twd)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, twd);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, fip)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, fip);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, fcs)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, fcs);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, foo)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, foo);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, fos)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, fos);
	}
	if (size > offsetof(TRACEE_REGS_STRUCT, st_space)) {
		const size_t len =
			size - offsetof(TRACEE_REGS_STRUCT, st_space);
		fputs(", st_space=[", stdout);
		for (unsigned int i = 0; i < ARRAY_SIZE(regs->st_space); ++i) {
			if (len > i * sizeof(regs->st_space[i])) {
				if (i)
					fputs(", ", stdout);
				PRINT_VAL_X(regs->st_space[i]);
			}
		}
		fputs("]", stdout);
	}

# elif defined __x86_64__

	PRINT_FIELD_X(*regs, cwd);
	if (size >= offsetofend(TRACEE_REGS_STRUCT, swd)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, swd);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, ftw)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, ftw);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, fop)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, fop);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, rip)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, rip);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, rdp)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, rdp);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, mxcsr)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, mxcsr);
	}
	if (size >= offsetofend(TRACEE_REGS_STRUCT, mxcr_mask)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, mxcr_mask);
	}
	if (size > offsetof(TRACEE_REGS_STRUCT, st_space)) {
		const size_t len =
			size - offsetof(TRACEE_REGS_STRUCT, st_space);
		fputs(", st_space=[", stdout);
		for (unsigned int i = 0; i < ARRAY_SIZE(regs->st_space); ++i) {
			if (len > i * sizeof(regs->st_space[i])) {
				if (i)
					fputs(", ", stdout);
				PRINT_VAL_X(regs->st_space[i]);
			}
		}
		fputs("]", stdout);
	}
	if (size > offsetof(TRACEE_REGS_STRUCT, xmm_space)) {
		const size_t len =
			size - offsetof(TRACEE_REGS_STRUCT, xmm_space);
		fputs(", xmm_space=[", stdout);
		for (unsigned int i = 0; i < ARRAY_SIZE(regs->xmm_space); ++i) {
			if (len > i * sizeof(regs->xmm_space[i])) {
				if (i)
					fputs(", ", stdout);
				PRINT_VAL_X(regs->xmm_space[i]);
			}
		}
		fputs("]", stdout);
	}
	if (size > offsetof(TRACEE_REGS_STRUCT, padding)) {
		const size_t len =
			size - offsetof(TRACEE_REGS_STRUCT, padding);
		fputs(", padding=[", stdout);
		for (unsigned int i = 0; i < ARRAY_SIZE(regs->padding); ++i) {
			if (len > i * sizeof(regs->padding[i])) {
				if (i)
					fputs(", ", stdout);
				PRINT_VAL_X(regs->padding[i]);
			}
		}
		fputs("]", stdout);
	}

# elif defined __powerpc__ || defined __powerpc64__

	fputs("fpr=[", stdout);
	for (unsigned int i = 0; i < ARRAY_SIZE(regs->fpr); ++i) {
		if (size > i * sizeof(regs->fpr[i])) {
			if (i)
				fputs(", ", stdout);
			PRINT_VAL_X(regs->fpr[i]);
		}
	}
	fputs("]", stdout);
	if (size >= offsetofend(TRACEE_REGS_STRUCT, fpscr)) {
		fputs(", ", stdout);
		PRINT_FIELD_X(*regs, fpscr);
	}

# endif /*
	   __i386__ ||
	   __powerpc64__ ||
	   __powerpc__ ||
	   __x86_64__
	 */

	if (size > sizeof(*regs))
		fputs(", ...", stdout);
	fputs("}", stdout);

#else /* !TRACEE_REGS_STRUCT */

	printf("%p", rs);

#endif /* TRACEE_REGS_STRUCT */
}

#ifdef PTRACE_GETFPREGS
static void
print_pt_fpregs(const void *const rs, const size_t size)
{
# ifdef TRACEE_REGS_STRUCT

	if (size != sizeof(TRACEE_REGS_STRUCT))
		error_msg_and_fail("expected size %zu, got size %zu",
				   sizeof(TRACEE_REGS_STRUCT), size);
	print_fpregset(rs, size);

# else /* !TRACEE_REGS_STRUCT */

	printf("%p", rs);

# endif
}

static void
do_getfpregs_setfpregs(const int pid,
		       void *const regbuf,
		       const unsigned int regsize,
		       unsigned int *const actual_size)
{
	if (do_ptrace_regs(PTRACE_GETFPREGS, pid, (uintptr_t) regbuf)) {
		printf("ptrace(PTRACE_GETFPREGS, %d, %p) = %s\n",
		       pid, regbuf, errstr);
		return;	/* skip PTRACE_SETFPREGS */
	} else {
		printf("ptrace(PTRACE_GETFPREGS, %d, ", pid);
		print_pt_fpregs(regbuf, regsize);
		printf(") = %s\n", errstr);
		if (*actual_size)
			return;	/* skip PTRACE_SETFPREGS */
		else
			*actual_size = regsize;
	}

	do_ptrace_regs(PTRACE_SETFPREGS, pid, (uintptr_t) regbuf);
	printf("ptrace(PTRACE_SETFPREGS, %d, ", pid);
	print_pt_fpregs(regbuf, regsize);
	printf(") = %s\n", errstr);
}
#endif /* PTRACE_GETFPREGS */

static void
do_getregset_setregset(const int pid,
		       const unsigned int nt,
		       const char *const nt_str,
		       void *const regbuf,
		       const unsigned int regsize,
		       unsigned int *actual_size,
		       struct iovec *const iov,
		       void (*print_regset_fn)(const void *, size_t))
{
	iov->iov_base = regbuf;
	iov->iov_len = regsize;
	do_ptrace(PTRACE_GETREGSET, pid, nt, (uintptr_t) iov);
	if (iov->iov_len == regsize) {
		printf("ptrace(PTRACE_GETREGSET, %d, %s"
		       ", {iov_base=", pid, nt_str);
		print_regset_fn(iov->iov_base, iov->iov_len);
		printf(", iov_len=%lu}) = %s\n",
		       (unsigned long) iov->iov_len, errstr);
		if (*actual_size)
			return;	/* skip PTRACE_SETREGSET */
	} else {
		printf("ptrace(PTRACE_GETREGSET, %d, %s"
		       ", {iov_base=", pid, nt_str);
		print_regset_fn(iov->iov_base, iov->iov_len);
		printf(", iov_len=%u => %lu}) = %s\n",
		       regsize, (unsigned long) iov->iov_len, errstr);
		if (*actual_size)
			error_msg_and_fail("iov_len changed again"
					   " from %u to %lu", regsize,
					   (unsigned long) iov->iov_len);
		*actual_size = iov->iov_len;
	}

#if defined __sparc__ && !defined __arch64__
	/*
	 * On sparc32 PTRACE_SETREGSET NT_PRSTATUS of size greater than 120
	 * has interesting side effects.
	 */
	if (nt == 1 && regsize > 120)
		return;
#endif /* __sparc__ && !__arch64__ */

	iov->iov_len = regsize;
	do_ptrace(PTRACE_SETREGSET, pid, nt, (uintptr_t) iov);
	if (iov->iov_len == regsize) {
		printf("ptrace(PTRACE_SETREGSET, %d, %s"
		       ", {iov_base=", pid, nt_str);
		print_regset_fn(iov->iov_base, regsize);
		printf(", iov_len=%lu}) = %s\n",
		       (unsigned long) iov->iov_len, errstr);
	} else {
		printf("ptrace(PTRACE_SETREGSET, %d, %s"
		       ", {iov_base=", pid, nt_str);
		print_regset_fn(iov->iov_base, regsize);
		printf(", iov_len=%u => %lu}) = %s\n",
		       regsize, (unsigned long) iov->iov_len, errstr);
	}
}

static void
test_getregset_setregset(int pid)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct iovec, iov);
	unsigned long addr = (uintptr_t) (iov + 1);

	iov->iov_base = (void *) addr;
	iov->iov_len = sizeof(kernel_ulong_t);

	do_ptrace(PTRACE_GETREGSET, pid, 1, (uintptr_t) iov);
	printf("ptrace(PTRACE_GETREGSET, %d, NT_PRSTATUS"
	       ", {iov_base=%p, iov_len=%lu}) = %s\n",
	       pid, iov->iov_base, (unsigned long) iov->iov_len, errstr);

	do_ptrace(PTRACE_SETREGSET, pid, 3, (uintptr_t) iov);
	printf("ptrace(PTRACE_SETREGSET, %d, NT_PRPSINFO"
	       ", {iov_base=%p, iov_len=%lu}) = %s\n",
	       pid, iov->iov_base, (unsigned long) iov->iov_len, errstr);

#ifdef PTRACE_GETREGS
	do_ptrace_regs(PTRACE_GETREGS, pid, addr);
	printf("ptrace(PTRACE_GETREGS, %d, %#lx) = %s\n",
	       pid, addr, errstr);
#endif
#ifdef PTRACE_SETREGS
	do_ptrace_regs(PTRACE_SETREGS, pid, addr);
	printf("ptrace(PTRACE_SETREGS, %d, %#lx) = %s\n",
	       pid, addr, errstr);
#endif
#ifdef PTRACE_GETFPREGS
	do_ptrace_regs(PTRACE_GETFPREGS, pid, addr);
	printf("ptrace(PTRACE_GETFPREGS, %d, %#lx) = %s\n",
	       pid, addr, errstr);
#endif
#ifdef PTRACE_SETFPREGS
	do_ptrace_regs(PTRACE_SETFPREGS, pid, addr);
	printf("ptrace(PTRACE_SETFPREGS, %d, %#lx) = %s\n",
	       pid, addr, errstr);
#endif

	for (; addr > (uintptr_t) iov; --addr) {
		do_ptrace(PTRACE_GETREGSET, pid, 1, addr);
		printf("ptrace(PTRACE_GETREGSET, %d, NT_PRSTATUS, %#lx) = %s\n",
		       pid, addr, errstr);

		do_ptrace(PTRACE_SETREGSET, pid, 2, addr);
		printf("ptrace(PTRACE_SETREGSET, %d, NT_FPREGSET, %#lx) = %s\n",
		       pid, addr, errstr);
	}

	pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		if (do_ptrace(PTRACE_TRACEME, 0, 0, 0))
			perror_msg_and_fail("child: PTRACE_TRACEME");

		raise(SIGSTOP);
		if (chdir("")) {
			;
		}
		_exit(0);
	}

	const unsigned int regset_buf_size = 4096 - 64;
	char *const regset_buf_endptr = midtail_alloc(0, regset_buf_size);

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
			error_msg_and_fail("unexpected exit status %#x",
					   WEXITSTATUS(status));
		}
		if (WIFSIGNALED(status))
			error_msg_and_fail("unexpected signal %u",
					   WTERMSIG(status));
		if (!WIFSTOPPED(status)) {
			kill(pid, SIGKILL);
			error_msg_and_fail("unexpected wait status %#x",
					   status);
		}

		static unsigned int actual_prstatus_size;
		for (unsigned int i = actual_prstatus_size;
		     i <= regset_buf_size &&
		     (!actual_prstatus_size ||
		      actual_prstatus_size + 1 >= i);
		     ++i) {
			do_getregset_setregset(pid, 1, "NT_PRSTATUS",
					       regset_buf_endptr - i, i,
					       &actual_prstatus_size,
					       iov, print_prstatus_regset);
		}
		/*
		 * In an unlikely case NT_PRSTATUS is not supported,
		 * use regset_buf_size.
		 */
		if (!actual_prstatus_size)
			actual_prstatus_size = regset_buf_size;

		static unsigned int actual_fpregset_size;
		for (unsigned int i = actual_fpregset_size;
		     i <= regset_buf_size &&
		     (!actual_fpregset_size ||
		      actual_fpregset_size + 1 >= i);
		     ++i) {
			do_getregset_setregset(pid, 2, "NT_FPREGSET",
					       regset_buf_endptr - i, i,
					       &actual_fpregset_size,
					       iov, print_fpregset);
		}
		/*
		 * In an unlikely case NT_FPREGSET is not supported,
		 * use regset_buf_size.
		 */
		if (!actual_fpregset_size)
			actual_fpregset_size = regset_buf_size;

#ifdef PTRACE_GETREGS
		static unsigned int actual_pt_regs_size
# ifdef __mips__
			/*
			 * For some reason MIPS kernels do not report
			 * PTRACE_GETREGS EFAULT errors.
			 */
			= 38 * 8
# endif
			;
		for (unsigned int i = actual_pt_regs_size;
		     i <= (actual_pt_regs_size ?: regset_buf_size);
		     ++i) {
			do_getregs_setregs(pid, regset_buf_endptr - i, i,
					   &actual_pt_regs_size);
		}
		/*
		 * In an unlikely case PTRACE_GETREGS is not supported,
		 * use regset_buf_size.
		 */
		if (!actual_pt_regs_size)
			actual_pt_regs_size = regset_buf_size;
#endif

#ifdef PTRACE_GETREGS64
		static unsigned int actual_pt_regs64_size;
		for (unsigned int i = actual_pt_regs64_size;
		     i <= (actual_pt_regs64_size ?: regset_buf_size);
		     ++i) {
			do_getregs64_setregs64(pid, regset_buf_endptr - i, i,
					       &actual_pt_regs64_size);
		}
		/*
		 * In an unlikely case PTRACE_GETREGS64 is not supported,
		 * use regset_buf_size.
		 */
		if (!actual_pt_regs64_size)
			actual_pt_regs64_size = regset_buf_size;
#endif

#ifdef PTRACE_GETFPREGS
		static unsigned int actual_pt_fpregs_size
# ifdef __mips__
			/*
			 * For some reason MIPS kernels do not report
			 * PTRACE_GETFPREGS EFAULT errors.
			 */
			= 33 * 8
# endif
			;
		for (unsigned int i = actual_pt_fpregs_size;
		     i <= (actual_pt_fpregs_size ?: regset_buf_size);
		     ++i) {
			do_getfpregs_setfpregs(pid, regset_buf_endptr - i, i,
					       &actual_pt_fpregs_size);
		}
		/*
		 * In an unlikely case PTRACE_GETFPREGS is not supported,
		 * use regset_buf_size.
		 */
		if (!actual_pt_fpregs_size)
			actual_pt_fpregs_size = regset_buf_size;
#endif

		if (WSTOPSIG(status) == SIGSTOP)
			kill(pid, SIGCONT);

		if (do_ptrace(PTRACE_SYSCALL, pid, 0, 0)) {
			saved = errno;
			kill(pid, SIGKILL);
			errno = saved;
			perror_msg_and_fail("ptrace");
		}
		printf("ptrace(PTRACE_SYSCALL, %d, NULL, 0) = 0\n", pid);
	}
}

int
main(void)
{
	const unsigned long bad_request =
		(unsigned long) 0xdeadbeeffffffeedULL;
	const unsigned long bad_data =
		(unsigned long) 0xdeadcafefffff00dULL;
	const int pid = getpid();

	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, filter_off);

	const unsigned int sigset_size = get_sigset_size();

	void *const k_set = tail_alloc(sigset_size);
	TAIL_ALLOC_OBJECT_CONST_PTR(siginfo_t, sip);

	do_ptrace(bad_request, pid, 0, 0);
	printf("ptrace(%#lx /* PTRACE_??? */, %d, NULL, NULL) = %s\n",
	       bad_request, pid, errstr);

	do_ptrace(PTRACE_PEEKDATA, pid, bad_request, bad_data);
#ifdef IA64
	printf("ptrace(PTRACE_PEEKDATA, %d, %#lx) = %s\n",
	       pid, bad_request, errstr);
#else
	printf("ptrace(PTRACE_PEEKDATA, %d, %#lx, %#lx) = %s\n",
	       pid, bad_request, bad_data, errstr);
#endif

	do_ptrace(PTRACE_PEEKTEXT, pid, bad_request, bad_data);
#ifdef IA64
	printf("ptrace(PTRACE_PEEKTEXT, %d, %#lx) = %s\n",
	       pid, bad_request, errstr);
#else
	printf("ptrace(PTRACE_PEEKTEXT, %d, %#lx, %#lx) = %s\n",
	       pid, bad_request, bad_data, errstr);
#endif

	do_ptrace(PTRACE_PEEKUSER, pid, bad_request, bad_data);
#ifdef IA64
	printf("ptrace(PTRACE_PEEKUSER, %d, %#lx) = %s\n",
	       pid, bad_request, errstr);
#else
	printf("ptrace(PTRACE_PEEKUSER, %d, %#lx, %#lx) = %s\n",
	       pid, bad_request, bad_data, errstr);
#endif

	do_ptrace(PTRACE_POKEUSER, pid, bad_request, bad_data);
	printf("ptrace(PTRACE_POKEUSER, %d, %#lx, %#lx) = %s\n",
	       pid, bad_request, bad_data, errstr);

	do_ptrace(PTRACE_ATTACH, pid, 0, 0);
	printf("ptrace(PTRACE_ATTACH, %d) = %s\n", pid, errstr);

	do_ptrace(PTRACE_INTERRUPT, pid, 0, 0);
	printf("ptrace(PTRACE_INTERRUPT, %d) = %s\n", pid, errstr);

	do_ptrace(PTRACE_KILL, pid, 0, 0);
	printf("ptrace(PTRACE_KILL, %d) = %s\n", pid, errstr);

	do_ptrace(PTRACE_LISTEN, pid, 0, 0);
	printf("ptrace(PTRACE_LISTEN, %d) = %s\n", pid, errstr);

	sigset_t libc_set;
	sigemptyset(&libc_set);
	sigaddset(&libc_set, SIGUSR1);
	memcpy(k_set, &libc_set, sigset_size);

	do_ptrace(PTRACE_SETSIGMASK, pid, sigset_size, (uintptr_t) k_set);
	printf("ptrace(PTRACE_SETSIGMASK, %d, %u, [USR1]) = %s\n",
	       pid, sigset_size, errstr);

	do_ptrace(PTRACE_GETSIGMASK, pid, sigset_size, (uintptr_t) k_set);
	printf("ptrace(PTRACE_GETSIGMASK, %d, %u, %p) = %s\n",
	       pid, sigset_size, k_set, errstr);

	do_ptrace(PTRACE_SECCOMP_GET_FILTER, pid, 42, 0);
	printf("ptrace(PTRACE_SECCOMP_GET_FILTER, %d, 42, NULL) = %s\n",
	       pid, errstr);

	do_ptrace(PTRACE_SECCOMP_GET_METADATA, pid, bad_data, 0);
	printf("ptrace(PTRACE_SECCOMP_GET_METADATA, %d, %lu, NULL) = %s\n",
	       pid, bad_data, errstr);

	do_ptrace(PTRACE_SECCOMP_GET_METADATA, pid, 7,
		  (uintptr_t) filter_off);
	printf("ptrace(PTRACE_SECCOMP_GET_METADATA, %d, 7, %p) = %s\n",
	       pid, filter_off, errstr);

	*filter_off = 0xfacefeeddeadc0deULL;
	do_ptrace(PTRACE_SECCOMP_GET_METADATA, pid, bad_data,
		  (uintptr_t) filter_off);
	printf("ptrace(PTRACE_SECCOMP_GET_METADATA, %d, %lu, "
	       "{filter_off=%" PRIu64 "}) = %s\n",
	       pid, bad_data, *filter_off, errstr);

	do_ptrace(PTRACE_GETEVENTMSG, pid, bad_request, bad_data);
	printf("ptrace(PTRACE_GETEVENTMSG, %d, %#lx, %#lx) = %s\n",
	       pid, bad_request, bad_data, errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGIO;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_band = -2;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGIO"
	       ", si_code=POLL_IN, si_errno=ENOENT, si_band=-2}) = %s\n",
	       pid, bad_request, errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGTRAP;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_pid = 2;
	sip->si_uid = 3;
	sip->si_ptr = (void *) bad_request;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGTRAP"
	       ", si_code=TRAP_BRKPT, si_errno=ENOENT, si_pid=2, si_uid=3"
	       ", si_int=%d, si_ptr=%p}) = %s\n",
	       pid, bad_request, sip->si_int, sip->si_ptr, errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGILL;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_addr = (void *) (unsigned long) 0xfacefeeddeadbeefULL;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGILL"
	       ", si_code=ILL_ILLOPC, si_errno=ENOENT, si_addr=%p}) = %s\n",
	       pid, bad_request, sip->si_addr, errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGFPE;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_addr = (void *) (unsigned long) 0xfacefeeddeadbeefULL;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGFPE"
	       ", si_code=FPE_INTDIV, si_errno=ENOENT, si_addr=%p}) = %s\n",
	       pid, bad_request, sip->si_addr, errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGBUS;
	sip->si_code = 1;
	sip->si_errno = -2;
	sip->si_addr = (void *) (unsigned long) 0xfacefeeddeadbeefULL;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGBUS"
	       ", si_code=BUS_ADRALN, si_errno=%u, si_addr=%p}) = %s\n",
	       pid, bad_request, sip->si_errno, sip->si_addr, errstr);

	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGPROF;
	sip->si_code = 0xbadc0ded;
	sip->si_errno = -2;
	sip->si_pid = 0;
	sip->si_uid = 3;
	sip->si_ptr = 0;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGPROF"
	       ", si_code=%#x, si_errno=%u, si_pid=0, si_uid=3}) = %s\n",
	       pid, bad_request, sip->si_code, sip->si_errno, errstr);

#ifdef HAVE_SIGINFO_T_SI_SYSCALL
	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGSYS;
	sip->si_code = 1;
	sip->si_errno = ENOENT;
	sip->si_call_addr = (void *) (unsigned long) 0xfacefeeddeadbeefULL;
	sip->si_syscall = -1U;
	sip->si_arch = AUDIT_ARCH_X86_64;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGSYS"
	       ", si_code=SYS_SECCOMP, si_errno=ENOENT, si_call_addr=%p"
	       ", si_syscall=%u, si_arch=AUDIT_ARCH_X86_64}) = %s\n",
	       pid, bad_request, sip->si_call_addr, sip->si_syscall, errstr);

	sip->si_errno = 3141592653U;
	sip->si_call_addr = NULL;
	sip->si_syscall = __NR_read;
	sip->si_arch = 0xda7a1057;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGSYS"
	       ", si_code=SYS_SECCOMP, si_errno=%u, si_call_addr=NULL"
	       ", si_syscall=%u, si_arch=%#x /* AUDIT_ARCH_??? */})"
	       " = %s\n",
	       pid, bad_request, sip->si_errno, sip->si_syscall, sip->si_arch,
	       errstr);

# ifdef CUR_AUDIT_ARCH
	sip->si_arch = CUR_AUDIT_ARCH;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGSYS"
	       ", si_code=SYS_SECCOMP, si_errno=%u, si_call_addr=NULL"
	       ", si_syscall=__NR_read, si_arch=%s"
	       "}) = %s\n",
	       pid, bad_request, sip->si_errno,
	       sprintxval(audit_arch, CUR_AUDIT_ARCH, "AUDIT_ARCH_???"),
	       errstr);
# endif
# if defined(PERS0_AUDIT_ARCH)
	sip->si_arch = PERS0_AUDIT_ARCH;
	sip->si_syscall = PERS0__NR_gettid;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGSYS"
	       ", si_code=SYS_SECCOMP, si_errno=%u, si_call_addr=NULL"
	       ", si_syscall=%u /* gettid */, si_arch=%s"
	       "}) = %s\n",
	       pid, bad_request, sip->si_errno, PERS0__NR_gettid,
	       sprintxval(audit_arch, PERS0_AUDIT_ARCH, "AUDIT_ARCH_???"),
	       errstr);
# endif
# if defined(M32_AUDIT_ARCH)
	sip->si_arch = M32_AUDIT_ARCH;
	sip->si_syscall = M32__NR_gettid;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGSYS"
	       ", si_code=SYS_SECCOMP, si_errno=%u, si_call_addr=NULL"
	       ", si_syscall=%u /* gettid */, si_arch=%s"
	       "}) = %s\n",
	       pid, bad_request, sip->si_errno, M32__NR_gettid,
	       sprintxval(audit_arch, M32_AUDIT_ARCH, "AUDIT_ARCH_???"),
	       errstr);
# endif
# if defined(MX32_AUDIT_ARCH)
	sip->si_arch = MX32_AUDIT_ARCH;
	sip->si_syscall = MX32__NR_gettid;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGSYS"
	       ", si_code=SYS_SECCOMP, si_errno=%u, si_call_addr=NULL"
	       ", si_syscall=%u /* gettid */, si_arch=%s"
	       "}) = %s\n",
	       pid, bad_request, sip->si_errno, MX32__NR_gettid,
	       sprintxval(audit_arch, MX32_AUDIT_ARCH, "AUDIT_ARCH_???"),
	       errstr);
# endif
#endif

#if defined HAVE_SIGINFO_T_SI_TIMERID && defined HAVE_SIGINFO_T_SI_OVERRUN
	memset(sip, -1, sizeof(*sip));
	sip->si_signo = SIGHUP;
	sip->si_code = SI_TIMER;
	sip->si_errno = ENOENT;
	sip->si_timerid = 0xdeadbeef;
	sip->si_overrun = -1;
	sip->si_ptr = (void *) (unsigned long) 0xfacefeeddeadbeefULL;

	do_ptrace(PTRACE_SETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_SETSIGINFO, %d, %#lx, {si_signo=SIGHUP"
	       ", si_code=SI_TIMER, si_errno=ENOENT, si_timerid=%#x"
	       ", si_overrun=%d, si_int=%d, si_ptr=%p}) = %s\n",
	       pid, bad_request, sip->si_timerid, sip->si_overrun,
	       sip->si_int, sip->si_ptr, errstr);
#endif

	do_ptrace(PTRACE_GETSIGINFO, pid, bad_request, (uintptr_t) sip);
	printf("ptrace(PTRACE_GETSIGINFO, %d, %#lx, %p) = %s\n",
	       pid, bad_request, sip, errstr);

	do_ptrace(PTRACE_CONT, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_CONT, %d, NULL, SIGUSR1) = %s\n",
	       pid, errstr);

	do_ptrace(PTRACE_DETACH, pid, 0, SIGUSR2);
	printf("ptrace(PTRACE_DETACH, %d, NULL, SIGUSR2) = %s\n",
	       pid, errstr);

	do_ptrace(PTRACE_SYSCALL, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_SYSCALL, %d, NULL, SIGUSR1) = %s\n",
	       pid, errstr);

#ifdef PTRACE_SINGLESTEP
	do_ptrace(PTRACE_SINGLESTEP, pid, 0, SIGUSR2);
	printf("ptrace(PTRACE_SINGLESTEP, %d, NULL, SIGUSR2) = %s\n",
	       pid, errstr);
#endif

#ifdef PTRACE_SINGLEBLOCK
	do_ptrace(PTRACE_SINGLEBLOCK, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_SINGLEBLOCK, %d, NULL, SIGUSR1) = %s\n",
	       pid, errstr);
#endif

#ifdef PTRACE_SYSEMU
	do_ptrace(PTRACE_SYSEMU, pid, 0, SIGUSR2);
	printf("ptrace(PTRACE_SYSEMU, %d, NULL, SIGUSR2) = %s\n",
	       pid, errstr);
#endif
#ifdef PTRACE_SYSEMU_SINGLESTEP
	do_ptrace(PTRACE_SYSEMU_SINGLESTEP, pid, 0, SIGUSR1);
	printf("ptrace(PTRACE_SYSEMU_SINGLESTEP, %d, NULL, SIGUSR1) = %s\n",
	       pid, errstr);
#endif

	do_ptrace(PTRACE_SETOPTIONS,
		  pid, 0, PTRACE_O_TRACEFORK|PTRACE_O_TRACECLONE);
	printf("ptrace(PTRACE_SETOPTIONS, %d, NULL"
	       ", PTRACE_O_TRACEFORK|PTRACE_O_TRACECLONE) = %s\n",
	       pid, errstr);

	do_ptrace(PTRACE_SEIZE, pid, bad_request, PTRACE_O_TRACESYSGOOD);
	printf("ptrace(PTRACE_SEIZE, %d, %#lx, PTRACE_O_TRACESYSGOOD) = %s\n",
	       pid, bad_request, errstr);

	test_peeksiginfo(pid, bad_request);
	test_getregset_setregset(pid);

	do_ptrace(PTRACE_TRACEME, 0, 0, 0);
	printf("ptrace(PTRACE_TRACEME) = %s\n", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
