/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
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

#include "defs.h"
#include <sys/user.h>
#include <sys/param.h>

#ifdef HAVE_SYS_REG_H
# include <sys/reg.h>
# ifndef PTRACE_PEEKUSR
#  define PTRACE_PEEKUSR PTRACE_PEEKUSER
# endif
#elif defined(HAVE_LINUX_PTRACE_H)
# undef PTRACE_SYSCALL
# ifdef HAVE_STRUCT_IA64_FPREG
#  define ia64_fpreg XXX_ia64_fpreg
# endif
# ifdef HAVE_STRUCT_PT_ALL_USER_REGS
#  define pt_all_user_regs XXX_pt_all_user_regs
# endif
# include <linux/ptrace.h>
# undef ia64_fpreg
# undef pt_all_user_regs
#endif

#if defined(SPARC64)
# undef PTRACE_GETREGS
# define PTRACE_GETREGS PTRACE_GETREGS64
# undef PTRACE_SETREGS
# define PTRACE_SETREGS PTRACE_SETREGS64
#endif

#if defined(IA64)
# include <asm/ptrace_offsets.h>
# include <asm/rse.h>
#endif

#ifndef ERESTARTSYS
# define ERESTARTSYS	512
#endif
#ifndef ERESTARTNOINTR
# define ERESTARTNOINTR	513
#endif
#ifndef ERESTARTNOHAND
# define ERESTARTNOHAND	514	/* restart if no handler */
#endif
#ifndef ERESTART_RESTARTBLOCK
# define ERESTART_RESTARTBLOCK 516	/* restart by calling sys_restart_syscall */
#endif

#ifndef NSIG
# warning: NSIG is not defined, using 32
# define NSIG 32
#endif
#ifdef ARM
/* Ugh. Is this really correct? ARM has no RT signals?! */
# undef NSIG
# define NSIG 32
#endif

#include "syscall.h"

/* Define these shorthand notations to simplify the syscallent files. */
#define TD TRACE_DESC
#define TF TRACE_FILE
#define TI TRACE_IPC
#define TN TRACE_NETWORK
#define TP TRACE_PROCESS
#define TS TRACE_SIGNAL
#define NF SYSCALL_NEVER_FAILS
#define MA MAX_ARGS

static const struct sysent sysent0[] = {
#include "syscallent.h"
};

#if SUPPORTED_PERSONALITIES >= 2
static const struct sysent sysent1[] = {
# include "syscallent1.h"
};
#endif

#if SUPPORTED_PERSONALITIES >= 3
static const struct sysent sysent2[] = {
# include "syscallent2.h"
};
#endif

/* Now undef them since short defines cause wicked namespace pollution. */
#undef TD
#undef TF
#undef TI
#undef TN
#undef TP
#undef TS
#undef NF
#undef MA

/*
 * `ioctlent.h' may be generated from `ioctlent.raw' by the auxiliary
 * program `ioctlsort', such that the list is sorted by the `code' field.
 * This has the side-effect of resolving the _IO.. macros into
 * plain integers, eliminating the need to include here everything
 * in "/usr/include".
 */

static const char *const errnoent0[] = {
#include "errnoent.h"
};
static const char *const signalent0[] = {
#include "signalent.h"
};
static const struct ioctlent ioctlent0[] = {
#include "ioctlent.h"
};
enum { nsyscalls0 = ARRAY_SIZE(sysent0) };
enum { nerrnos0 = ARRAY_SIZE(errnoent0) };
enum { nsignals0 = ARRAY_SIZE(signalent0) };
enum { nioctlents0 = ARRAY_SIZE(ioctlent0) };
int qual_flags0[MAX_QUALS];

#if SUPPORTED_PERSONALITIES >= 2
static const char *const errnoent1[] = {
# include "errnoent1.h"
};
static const char *const signalent1[] = {
# include "signalent1.h"
};
static const struct ioctlent ioctlent1[] = {
# include "ioctlent1.h"
};
enum { nsyscalls1 = ARRAY_SIZE(sysent1) };
enum { nerrnos1 = ARRAY_SIZE(errnoent1) };
enum { nsignals1 = ARRAY_SIZE(signalent1) };
enum { nioctlents1 = ARRAY_SIZE(ioctlent1) };
int qual_flags1[MAX_QUALS];
#endif

#if SUPPORTED_PERSONALITIES >= 3
static const char *const errnoent2[] = {
# include "errnoent2.h"
};
static const char *const signalent2[] = {
# include "signalent2.h"
};
static const struct ioctlent ioctlent2[] = {
# include "ioctlent2.h"
};
enum { nsyscalls2 = ARRAY_SIZE(sysent2) };
enum { nerrnos2 = ARRAY_SIZE(errnoent2) };
enum { nsignals2 = ARRAY_SIZE(signalent2) };
enum { nioctlents2 = ARRAY_SIZE(ioctlent2) };
int qual_flags2[MAX_QUALS];
#endif

const struct sysent *sysent = sysent0;
const char *const *errnoent = errnoent0;
const char *const *signalent = signalent0;
const struct ioctlent *ioctlent = ioctlent0;
unsigned nsyscalls = nsyscalls0;
unsigned nerrnos = nerrnos0;
unsigned nsignals = nsignals0;
unsigned nioctlents = nioctlents0;
int *qual_flags = qual_flags0;

#if SUPPORTED_PERSONALITIES > 1
int current_personality;

const int personality_wordsize[SUPPORTED_PERSONALITIES] = {
	PERSONALITY0_WORDSIZE,
	PERSONALITY1_WORDSIZE,
# if SUPPORTED_PERSONALITIES > 2
	PERSONALITY2_WORDSIZE,
# endif
};

void
set_personality(int personality)
{
	switch (personality) {
	case 0:
		errnoent = errnoent0;
		nerrnos = nerrnos0;
		sysent = sysent0;
		nsyscalls = nsyscalls0;
		ioctlent = ioctlent0;
		nioctlents = nioctlents0;
		signalent = signalent0;
		nsignals = nsignals0;
		qual_flags = qual_flags0;
		break;

	case 1:
		errnoent = errnoent1;
		nerrnos = nerrnos1;
		sysent = sysent1;
		nsyscalls = nsyscalls1;
		ioctlent = ioctlent1;
		nioctlents = nioctlents1;
		signalent = signalent1;
		nsignals = nsignals1;
		qual_flags = qual_flags1;
		break;

# if SUPPORTED_PERSONALITIES >= 3
	case 2:
		errnoent = errnoent2;
		nerrnos = nerrnos2;
		sysent = sysent2;
		nsyscalls = nsyscalls2;
		ioctlent = ioctlent2;
		nioctlents = nioctlents2;
		signalent = signalent2;
		nsignals = nsignals2;
		qual_flags = qual_flags2;
		break;
# endif
	}

	current_personality = personality;
}

static void
update_personality(struct tcb *tcp, int personality)
{
	if (personality == current_personality)
		return;
	set_personality(personality);

	if (personality == tcp->currpers)
		return;
	tcp->currpers = personality;

# if defined(POWERPC64)
	if (!qflag) {
		static const char *const names[] = {"64 bit", "32 bit"};
		fprintf(stderr, "[ Process PID=%d runs in %s mode. ]\n",
			tcp->pid, names[personality]);
	}
# elif defined(X86_64)
	if (!qflag) {
		static const char *const names[] = {"64 bit", "32 bit", "x32"};
		fprintf(stderr, "[ Process PID=%d runs in %s mode. ]\n",
			tcp->pid, names[personality]);
	}
# elif defined(X32)
	if (!qflag) {
		static const char *const names[] = {"x32", "32 bit"};
		fprintf(stderr, "[ Process PID=%d runs in %s mode. ]\n",
			tcp->pid, names[personality]);
	}
# endif
}
#endif

static int qual_syscall(), qual_signal(), qual_fault(), qual_desc();

static const struct qual_options {
	int bitflag;
	const char *option_name;
	int (*qualify)(const char *, int, int);
	const char *argument_name;
} qual_options[] = {
	{ QUAL_TRACE,	"trace",	qual_syscall,	"system call"	},
	{ QUAL_TRACE,	"t",		qual_syscall,	"system call"	},
	{ QUAL_ABBREV,	"abbrev",	qual_syscall,	"system call"	},
	{ QUAL_ABBREV,	"a",		qual_syscall,	"system call"	},
	{ QUAL_VERBOSE,	"verbose",	qual_syscall,	"system call"	},
	{ QUAL_VERBOSE,	"v",		qual_syscall,	"system call"	},
	{ QUAL_RAW,	"raw",		qual_syscall,	"system call"	},
	{ QUAL_RAW,	"x",		qual_syscall,	"system call"	},
	{ QUAL_SIGNAL,	"signal",	qual_signal,	"signal"	},
	{ QUAL_SIGNAL,	"signals",	qual_signal,	"signal"	},
	{ QUAL_SIGNAL,	"s",		qual_signal,	"signal"	},
	{ QUAL_FAULT,	"fault",	qual_fault,	"fault"		},
	{ QUAL_FAULT,	"faults",	qual_fault,	"fault"		},
	{ QUAL_FAULT,	"m",		qual_fault,	"fault"		},
	{ QUAL_READ,	"read",		qual_desc,	"descriptor"	},
	{ QUAL_READ,	"reads",	qual_desc,	"descriptor"	},
	{ QUAL_READ,	"r",		qual_desc,	"descriptor"	},
	{ QUAL_WRITE,	"write",	qual_desc,	"descriptor"	},
	{ QUAL_WRITE,	"writes",	qual_desc,	"descriptor"	},
	{ QUAL_WRITE,	"w",		qual_desc,	"descriptor"	},
	{ 0,		NULL,		NULL,		NULL		},
};

static void
qualify_one(int n, int bitflag, int not, int pers)
{
	if (pers == 0 || pers < 0) {
		if (not)
			qual_flags0[n] &= ~bitflag;
		else
			qual_flags0[n] |= bitflag;
	}

#if SUPPORTED_PERSONALITIES >= 2
	if (pers == 1 || pers < 0) {
		if (not)
			qual_flags1[n] &= ~bitflag;
		else
			qual_flags1[n] |= bitflag;
	}
#endif

#if SUPPORTED_PERSONALITIES >= 3
	if (pers == 2 || pers < 0) {
		if (not)
			qual_flags2[n] &= ~bitflag;
		else
			qual_flags2[n] |= bitflag;
	}
#endif
}

static int
qual_syscall(const char *s, int bitflag, int not)
{
	int i;
	int rc = -1;

	if (*s >= '0' && *s <= '9') {
		int i = string_to_uint(s);
		if (i < 0 || i >= MAX_QUALS)
			return -1;
		qualify_one(i, bitflag, not, -1);
		return 0;
	}
	for (i = 0; i < nsyscalls0; i++)
		if (sysent0[i].sys_name &&
		    strcmp(s, sysent0[i].sys_name) == 0) {
			qualify_one(i, bitflag, not, 0);
			rc = 0;
		}

#if SUPPORTED_PERSONALITIES >= 2
	for (i = 0; i < nsyscalls1; i++)
		if (sysent1[i].sys_name &&
		    strcmp(s, sysent1[i].sys_name) == 0) {
			qualify_one(i, bitflag, not, 1);
			rc = 0;
		}
#endif

#if SUPPORTED_PERSONALITIES >= 3
	for (i = 0; i < nsyscalls2; i++)
		if (sysent2[i].sys_name &&
		    strcmp(s, sysent2[i].sys_name) == 0) {
			qualify_one(i, bitflag, not, 2);
			rc = 0;
		}
#endif

	return rc;
}

static int
qual_signal(const char *s, int bitflag, int not)
{
	int i;

	if (*s >= '0' && *s <= '9') {
		int signo = string_to_uint(s);
		if (signo < 0 || signo >= MAX_QUALS)
			return -1;
		qualify_one(signo, bitflag, not, -1);
		return 0;
	}
	if (strncasecmp(s, "SIG", 3) == 0)
		s += 3;
	for (i = 0; i <= NSIG; i++) {
		if (strcasecmp(s, signame(i) + 3) == 0) {
			qualify_one(i, bitflag, not, -1);
			return 0;
		}
	}
	return -1;
}

static int
qual_fault(const char *s, int bitflag, int not)
{
	return -1;
}

static int
qual_desc(const char *s, int bitflag, int not)
{
	if (*s >= '0' && *s <= '9') {
		int desc = string_to_uint(s);
		if (desc < 0 || desc >= MAX_QUALS)
			return -1;
		qualify_one(desc, bitflag, not, -1);
		return 0;
	}
	return -1;
}

static int
lookup_class(const char *s)
{
	if (strcmp(s, "file") == 0)
		return TRACE_FILE;
	if (strcmp(s, "ipc") == 0)
		return TRACE_IPC;
	if (strcmp(s, "network") == 0)
		return TRACE_NETWORK;
	if (strcmp(s, "process") == 0)
		return TRACE_PROCESS;
	if (strcmp(s, "signal") == 0)
		return TRACE_SIGNAL;
	if (strcmp(s, "desc") == 0)
		return TRACE_DESC;
	return -1;
}

void
qualify(const char *s)
{
	const struct qual_options *opt;
	int not;
	char *copy;
	const char *p;
	int i, n;

	opt = &qual_options[0];
	for (i = 0; (p = qual_options[i].option_name); i++) {
		n = strlen(p);
		if (strncmp(s, p, n) == 0 && s[n] == '=') {
			opt = &qual_options[i];
			s += n + 1;
			break;
		}
	}
	not = 0;
	if (*s == '!') {
		not = 1;
		s++;
	}
	if (strcmp(s, "none") == 0) {
		not = 1 - not;
		s = "all";
	}
	if (strcmp(s, "all") == 0) {
		for (i = 0; i < MAX_QUALS; i++) {
			qualify_one(i, opt->bitflag, not, -1);
		}
		return;
	}
	for (i = 0; i < MAX_QUALS; i++) {
		qualify_one(i, opt->bitflag, !not, -1);
	}
	copy = strdup(s);
	if (!copy)
		die_out_of_memory();
	for (p = strtok(copy, ","); p; p = strtok(NULL, ",")) {
		if (opt->bitflag == QUAL_TRACE && (n = lookup_class(p)) > 0) {
			for (i = 0; i < nsyscalls0; i++)
				if (sysent0[i].sys_flags & n)
					qualify_one(i, opt->bitflag, not, 0);

#if SUPPORTED_PERSONALITIES >= 2
			for (i = 0; i < nsyscalls1; i++)
				if (sysent1[i].sys_flags & n)
					qualify_one(i, opt->bitflag, not, 1);
#endif

#if SUPPORTED_PERSONALITIES >= 3
			for (i = 0; i < nsyscalls2; i++)
				if (sysent2[i].sys_flags & n)
					qualify_one(i, opt->bitflag, not, 2);
#endif

			continue;
		}
		if (opt->qualify(p, opt->bitflag, not)) {
			error_msg_and_die("invalid %s '%s'",
				opt->argument_name, p);
		}
	}
	free(copy);
	return;
}

#ifdef SYS_socket_subcall
static void
decode_socket_subcall(struct tcb *tcp)
{
	unsigned long addr;
	unsigned int i, size;

	if (tcp->u_arg[0] < 0 || tcp->u_arg[0] >= SYS_socket_nsubcalls)
		return;

	tcp->scno = SYS_socket_subcall + tcp->u_arg[0];
	addr = tcp->u_arg[1];
	tcp->u_nargs = sysent[tcp->scno].nargs;
	size = current_wordsize;
	for (i = 0; i < tcp->u_nargs; ++i) {
		if (size == sizeof(int)) {
			unsigned int arg;
			if (umove(tcp, addr, &arg) < 0)
				arg = 0;
			tcp->u_arg[i] = arg;
		}
		else {
			unsigned long arg;
			if (umove(tcp, addr, &arg) < 0)
				arg = 0;
			tcp->u_arg[i] = arg;
		}
		addr += size;
	}
}
#endif

#ifdef SYS_ipc_subcall
static void
decode_ipc_subcall(struct tcb *tcp)
{
	unsigned int i;

	if (tcp->u_arg[0] < 0 || tcp->u_arg[0] >= SYS_ipc_nsubcalls)
		return;

	tcp->scno = SYS_ipc_subcall + tcp->u_arg[0];
	tcp->u_nargs = sysent[tcp->scno].nargs;
	for (i = 0; i < tcp->u_nargs; i++)
		tcp->u_arg[i] = tcp->u_arg[i + 1];
}
#endif

int
printargs(struct tcb *tcp)
{
	if (entering(tcp)) {
		int i;

		for (i = 0; i < tcp->u_nargs; i++)
			tprintf("%s%#lx", i ? ", " : "", tcp->u_arg[i]);
	}
	return 0;
}

int
printargs_lu(struct tcb *tcp)
{
	if (entering(tcp)) {
		int i;

		for (i = 0; i < tcp->u_nargs; i++)
			tprintf("%s%lu", i ? ", " : "", tcp->u_arg[i]);
	}
	return 0;
}

int
printargs_ld(struct tcb *tcp)
{
	if (entering(tcp)) {
		int i;

		for (i = 0; i < tcp->u_nargs; i++)
			tprintf("%s%ld", i ? ", " : "", tcp->u_arg[i]);
	}
	return 0;
}

long
getrval2(struct tcb *tcp)
{
	long val = -1;

#if defined(SPARC) || defined(SPARC64)
	struct pt_regs regs;
	if (ptrace(PTRACE_GETREGS, tcp->pid, (char *)&regs, 0) < 0)
		return -1;
	val = regs.u_regs[U_REG_O1];
#elif defined(SH)
	if (upeek(tcp, 4*(REG_REG0+1), &val) < 0)
		return -1;
#elif defined(IA64)
	if (upeek(tcp, PT_R9, &val) < 0)
		return -1;
#endif

	return val;
}

int
is_restart_error(struct tcb *tcp)
{
	switch (tcp->u_error) {
		case ERESTARTSYS:
		case ERESTARTNOINTR:
		case ERESTARTNOHAND:
		case ERESTART_RESTARTBLOCK:
			return 1;
		default:
			break;
	}
	return 0;
}

#if defined(I386)
struct pt_regs i386_regs;
#elif defined(X86_64) || defined(X32)
/*
 * On 32 bits, pt_regs and user_regs_struct are the same,
 * but on 64 bits, user_regs_struct has six more fields:
 * fs_base, gs_base, ds, es, fs, gs.
 * PTRACE_GETREGS fills them too, so struct pt_regs would overflow.
 */
static struct user_regs_struct x86_64_regs;
#elif defined(IA64)
long r8, r10, psr; /* TODO: make static? */
long ia32 = 0; /* not static */
#elif defined(POWERPC)
static long ppc_result;
#elif defined(M68K)
static long d0;
#elif defined(BFIN)
static long r0;
#elif defined(ARM)
static struct pt_regs regs;
#elif defined(ALPHA)
static long r0;
static long a3;
#elif defined(AVR32)
static struct pt_regs regs;
#elif defined(SPARC) || defined(SPARC64)
static struct pt_regs regs;
static unsigned long trap;
#elif defined(LINUX_MIPSN32)
static long long a3;
static long long r2;
#elif defined(MIPS)
static long a3;
static long r2;
#elif defined(S390) || defined(S390X)
static long gpr2;
static long pc;
static long syscall_mode;
#elif defined(HPPA)
static long r28;
#elif defined(SH)
static long r0;
#elif defined(SH64)
static long r9;
#elif defined(CRISV10) || defined(CRISV32)
static long r10;
#elif defined(MICROBLAZE)
static long r3;
#endif

/* Returns:
 * 0: "ignore this ptrace stop", bail out of trace_syscall_entering() silently.
 * 1: ok, continue in trace_syscall_entering().
 * other: error, trace_syscall_entering() should print error indicator
 *    ("????" etc) and bail out.
 */
static int
get_scno(struct tcb *tcp)
{
	long scno = 0;

#if defined(S390) || defined(S390X)
	if (upeek(tcp, PT_GPR2, &syscall_mode) < 0)
		return -1;

	if (syscall_mode != -ENOSYS) {
		/*
		 * Since kernel version 2.5.44 the scno gets passed in gpr2.
		 */
		scno = syscall_mode;
	} else {
		/*
		 * Old style of "passing" the scno via the SVC instruction.
		 */
		long opcode, offset_reg, tmp;
		void *svc_addr;
		static const int gpr_offset[16] = {
				PT_GPR0,  PT_GPR1,  PT_ORIGGPR2, PT_GPR3,
				PT_GPR4,  PT_GPR5,  PT_GPR6,     PT_GPR7,
				PT_GPR8,  PT_GPR9,  PT_GPR10,    PT_GPR11,
				PT_GPR12, PT_GPR13, PT_GPR14,    PT_GPR15
		};

		if (upeek(tcp, PT_PSWADDR, &pc) < 0)
			return -1;
		errno = 0;
		opcode = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)(pc-sizeof(long)), 0);
		if (errno) {
			perror("peektext(pc-oneword)");
			return -1;
		}

		/*
		 *  We have to check if the SVC got executed directly or via an
		 *  EXECUTE instruction. In case of EXECUTE it is necessary to do
		 *  instruction decoding to derive the system call number.
		 *  Unfortunately the opcode sizes of EXECUTE and SVC are differently,
		 *  so that this doesn't work if a SVC opcode is part of an EXECUTE
		 *  opcode. Since there is no way to find out the opcode size this
		 *  is the best we can do...
		 */
		if ((opcode & 0xff00) == 0x0a00) {
			/* SVC opcode */
			scno = opcode & 0xff;
		}
		else {
			/* SVC got executed by EXECUTE instruction */

			/*
			 *  Do instruction decoding of EXECUTE. If you really want to
			 *  understand this, read the Principles of Operations.
			 */
			svc_addr = (void *) (opcode & 0xfff);

			tmp = 0;
			offset_reg = (opcode & 0x000f0000) >> 16;
			if (offset_reg && (upeek(tcp, gpr_offset[offset_reg], &tmp) < 0))
				return -1;
			svc_addr += tmp;

			tmp = 0;
			offset_reg = (opcode & 0x0000f000) >> 12;
			if (offset_reg && (upeek(tcp, gpr_offset[offset_reg], &tmp) < 0))
				return -1;
			svc_addr += tmp;

			scno = ptrace(PTRACE_PEEKTEXT, tcp->pid, svc_addr, 0);
			if (errno)
				return -1;
# if defined(S390X)
			scno >>= 48;
# else
			scno >>= 16;
# endif
			tmp = 0;
			offset_reg = (opcode & 0x00f00000) >> 20;
			if (offset_reg && (upeek(tcp, gpr_offset[offset_reg], &tmp) < 0))
				return -1;

			scno = (scno | tmp) & 0xff;
		}
	}
#elif defined(POWERPC)
	if (upeek(tcp, sizeof(unsigned long)*PT_R0, &scno) < 0)
		return -1;
# ifdef POWERPC64
	/* TODO: speed up strace by not doing this at every syscall.
	 * We only need to do it after execve.
	 */
	int currpers;
	long val;

	/* Check for 64/32 bit mode. */
	if (upeek(tcp, sizeof(unsigned long)*PT_MSR, &val) < 0)
		return -1;
	/* SF is bit 0 of MSR */
	if (val < 0)
		currpers = 0;
	else
		currpers = 1;
	update_personality(tcp, currpers);
# endif
#elif defined(AVR32)
	/* Read complete register set in one go. */
	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, &regs) < 0)
		return -1;
	scno = regs.r8;
#elif defined(BFIN)
	if (upeek(tcp, PT_ORIG_P0, &scno))
		return -1;
#elif defined(I386)
	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long) &i386_regs) < 0)
		return -1;
	scno = i386_regs.orig_eax;
#elif defined(X86_64) || defined(X32)
# ifndef __X32_SYSCALL_BIT
#  define __X32_SYSCALL_BIT	0x40000000
# endif
# ifndef __X32_SYSCALL_MASK
#  define __X32_SYSCALL_MASK	__X32_SYSCALL_BIT
# endif

	int currpers;
	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long) &x86_64_regs) < 0)
		return -1;
	scno = x86_64_regs.orig_rax;

	/* Check CS register value. On x86-64 linux it is:
	 *	0x33	for long mode (64 bit)
	 *	0x23	for compatibility mode (32 bit)
	 * Check DS register value. On x86-64 linux it is:
	 *	0x2b	for x32 mode (x86-64 in 32 bit)
	 */
	switch (x86_64_regs.cs) {
		case 0x23: currpers = 1; break;
		case 0x33:
			if (x86_64_regs.ds == 0x2b) {
				currpers = 2;
				scno &= ~__X32_SYSCALL_MASK;
			} else
				currpers = 0;
			break;
		default:
			fprintf(stderr, "Unknown value CS=0x%08X while "
				 "detecting personality of process "
				 "PID=%d\n", (int)x86_64_regs.cs, tcp->pid);
			currpers = current_personality;
			break;
	}
# if 0
	/* This version analyzes the opcode of a syscall instruction.
	 * (int 0x80 on i386 vs. syscall on x86-64)
	 * It works, but is too complicated.
	 */
	unsigned long val, rip, i;

	rip = x86_64_regs.rip;

	/* sizeof(syscall) == sizeof(int 0x80) == 2 */
	rip -= 2;
	errno = 0;

	call = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)rip, (char *)0);
	if (errno)
		fprintf(stderr, "ptrace_peektext failed: %s\n",
				strerror(errno));
	switch (call & 0xffff) {
		/* x86-64: syscall = 0x0f 0x05 */
		case 0x050f: currpers = 0; break;
		/* i386: int 0x80 = 0xcd 0x80 */
		case 0x80cd: currpers = 1; break;
		default:
			currpers = current_personality;
			fprintf(stderr,
				"Unknown syscall opcode (0x%04X) while "
				"detecting personality of process "
				"PID=%d\n", (int)call, tcp->pid);
			break;
	}
# endif
# ifdef X32
	/* Value of currpers:
	 *   0: 64 bit
	 *   1: 32 bit
	 *   2: X32
	 * Value of current_personality:
	 *   0: X32
	 *   1: 32 bit
	 */
	switch (currpers) {
		case 0:
			fprintf(stderr, "syscall_%lu (...) in unsupported "
					"64-bit mode of process PID=%d\n",
				scno, tcp->pid);
			return 0;
		case 2:
			currpers = 0;
	}
# endif
	update_personality(tcp, currpers);
#elif defined(IA64)
#	define IA64_PSR_IS	((long)1 << 34)
	if (upeek(tcp, PT_CR_IPSR, &psr) >= 0)
		ia32 = (psr & IA64_PSR_IS) != 0;
	if (ia32) {
		if (upeek(tcp, PT_R1, &scno) < 0)
			return -1;
	} else {
		if (upeek(tcp, PT_R15, &scno) < 0)
			return -1;
	}
#elif defined(ARM)
	/* Read complete register set in one go. */
	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (void *)&regs) == -1)
		return -1;

	/*
	 * We only need to grab the syscall number on syscall entry.
	 */
	if (regs.ARM_ip == 0) {
		/*
		 * Note: we only deal with only 32-bit CPUs here.
		 */
		if (regs.ARM_cpsr & 0x20) {
			/*
			 * Get the Thumb-mode system call number
			 */
			scno = regs.ARM_r7;
		} else {
			/*
			 * Get the ARM-mode system call number
			 */
			errno = 0;
			scno = ptrace(PTRACE_PEEKTEXT, tcp->pid, (void *)(regs.ARM_pc - 4), NULL);
			if (errno)
				return -1;

			/* Handle the EABI syscall convention.  We do not
			   bother converting structures between the two
			   ABIs, but basic functionality should work even
			   if strace and the traced program have different
			   ABIs.  */
			if (scno == 0xef000000) {
				scno = regs.ARM_r7;
			} else {
				if ((scno & 0x0ff00000) != 0x0f900000) {
					fprintf(stderr, "syscall: unknown syscall trap 0x%08lx\n",
						scno);
					return -1;
				}

				/*
				 * Fixup the syscall number
				 */
				scno &= 0x000fffff;
			}
		}
		if (scno & 0x0f0000) {
			/*
			 * Handle ARM specific syscall
			 */
			update_personality(tcp, 1);
			scno &= 0x0000ffff;
		} else
			update_personality(tcp, 0);

	} else {
		fprintf(stderr, "pid %d stray syscall entry\n", tcp->pid);
		tcp->flags |= TCB_INSYSCALL;
	}
#elif defined(M68K)
	if (upeek(tcp, 4*PT_ORIG_D0, &scno) < 0)
		return -1;
#elif defined(LINUX_MIPSN32)
	unsigned long long regs[38];

	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long) &regs) < 0)
		return -1;
	a3 = regs[REG_A3];
	r2 = regs[REG_V0];

	scno = r2;
	if (!SCNO_IN_RANGE(scno)) {
		if (a3 == 0 || a3 == -1) {
			if (debug_flag)
				fprintf(stderr, "stray syscall exit: v0 = %ld\n", scno);
			return 0;
		}
	}
#elif defined(MIPS)
	if (upeek(tcp, REG_A3, &a3) < 0)
		return -1;
	if (upeek(tcp, REG_V0, &scno) < 0)
		return -1;

	if (!SCNO_IN_RANGE(scno)) {
		if (a3 == 0 || a3 == -1) {
			if (debug_flag)
				fprintf(stderr, "stray syscall exit: v0 = %ld\n", scno);
			return 0;
		}
	}
#elif defined(ALPHA)
	if (upeek(tcp, REG_A3, &a3) < 0)
		return -1;
	if (upeek(tcp, REG_R0, &scno) < 0)
		return -1;

	/*
	 * Do some sanity checks to figure out if it's
	 * really a syscall entry
	 */
	if (!SCNO_IN_RANGE(scno)) {
		if (a3 == 0 || a3 == -1) {
			if (debug_flag)
				fprintf(stderr, "stray syscall exit: r0 = %ld\n", scno);
			return 0;
		}
	}
#elif defined(SPARC) || defined(SPARC64)
	/* Everything we need is in the current register set. */
	if (ptrace(PTRACE_GETREGS, tcp->pid, (char *)&regs, 0) < 0)
		return -1;

	/* Disassemble the syscall trap. */
	/* Retrieve the syscall trap instruction. */
	errno = 0;
# if defined(SPARC64)
	trap = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)regs.tpc, 0);
	trap >>= 32;
# else
	trap = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)regs.pc, 0);
# endif
	if (errno)
		return -1;

	/* Disassemble the trap to see what personality to use. */
	switch (trap) {
	case 0x91d02010:
		/* Linux/SPARC syscall trap. */
		update_personality(tcp, 0);
		break;
	case 0x91d0206d:
		/* Linux/SPARC64 syscall trap. */
		update_personality(tcp, 2);
		break;
	case 0x91d02000:
		/* SunOS syscall trap. (pers 1) */
		fprintf(stderr, "syscall: SunOS no support\n");
		return -1;
	case 0x91d02008:
		/* Solaris 2.x syscall trap. (per 2) */
		update_personality(tcp, 1);
		break;
	case 0x91d02009:
		/* NetBSD/FreeBSD syscall trap. */
		fprintf(stderr, "syscall: NetBSD/FreeBSD not supported\n");
		return -1;
	case 0x91d02027:
		/* Solaris 2.x gettimeofday */
		update_personality(tcp, 1);
		break;
	default:
# if defined(SPARC64)
		fprintf(stderr, "syscall: unknown syscall trap %08lx %016lx\n", trap, regs.tpc);
# else
		fprintf(stderr, "syscall: unknown syscall trap %08lx %08lx\n", trap, regs.pc);
# endif
		return -1;
	}

	/* Extract the system call number from the registers. */
	if (trap == 0x91d02027)
		scno = 156;
	else
		scno = regs.u_regs[U_REG_G1];
	if (scno == 0) {
		scno = regs.u_regs[U_REG_O0];
		memmove(&regs.u_regs[U_REG_O0], &regs.u_regs[U_REG_O1], 7*sizeof(regs.u_regs[0]));
	}
#elif defined(HPPA)
	if (upeek(tcp, PT_GR20, &scno) < 0)
		return -1;
#elif defined(SH)
	/*
	 * In the new syscall ABI, the system call number is in R3.
	 */
	if (upeek(tcp, 4*(REG_REG0+3), &scno) < 0)
		return -1;

	if (scno < 0) {
		/* Odd as it may seem, a glibc bug has been known to cause
		   glibc to issue bogus negative syscall numbers.  So for
		   our purposes, make strace print what it *should* have been */
		long correct_scno = (scno & 0xff);
		if (debug_flag)
			fprintf(stderr,
				"Detected glibc bug: bogus system call"
				" number = %ld, correcting to %ld\n",
				scno,
				correct_scno);
		scno = correct_scno;
	}
#elif defined(SH64)
	if (upeek(tcp, REG_SYSCALL, &scno) < 0)
		return -1;
	scno &= 0xFFFF;
#elif defined(CRISV10) || defined(CRISV32)
	if (upeek(tcp, 4*PT_R9, &scno) < 0)
		return -1;
#elif defined(TILE)
	if (upeek(tcp, PTREGS_OFFSET_REG(10), &scno) < 0)
		return -1;
#elif defined(MICROBLAZE)
	if (upeek(tcp, 0, &scno) < 0)
		return -1;
#endif

#if defined(SH)
	/* new syscall ABI returns result in R0 */
	if (upeek(tcp, 4*REG_REG0, (long *)&r0) < 0)
		return -1;
#elif defined(SH64)
	/* ABI defines result returned in r9 */
	if (upeek(tcp, REG_GENERAL(9), (long *)&r9) < 0)
		return -1;
#endif

	tcp->scno = scno;
	return 1;
}

/* Called at each syscall entry.
 * Returns:
 * 0: "ignore this ptrace stop", bail out of trace_syscall_entering() silently.
 * 1: ok, continue in trace_syscall_entering().
 * other: error, trace_syscall_entering() should print error indicator
 *    ("????" etc) and bail out.
 */
static int
syscall_fixup_on_sysenter(struct tcb *tcp)
{
	/* A common case of "not a syscall entry" is post-execve SIGTRAP */
#if defined(I386)
	if (i386_regs.eax != -ENOSYS) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (eax = %ld)\n", i386_regs.eax);
		return 0;
	}
#elif defined(X86_64) || defined(X32)
	{
		long rax = x86_64_regs.rax;
		if (current_personality == 1)
			rax = (int)rax; /* sign extend from 32 bits */
		if (rax != -ENOSYS) {
			if (debug_flag)
				fprintf(stderr, "not a syscall entry (rax = %ld)\n", rax);
			return 0;
		}
	}
#elif defined(S390) || defined(S390X)
	/* TODO: we already fetched PT_GPR2 in get_scno
	 * and stored it in syscall_mode, reuse it here
	 * instead of re-fetching?
	 */
	if (upeek(tcp, PT_GPR2, &gpr2) < 0)
		return -1;
	if (syscall_mode != -ENOSYS)
		syscall_mode = tcp->scno;
	if (gpr2 != syscall_mode) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (gpr2 = %ld)\n", gpr2);
		return 0;
	}
#elif defined(M68K)
	/* TODO? Eliminate upeek's in arches below like we did in x86 */
	if (upeek(tcp, 4*PT_D0, &d0) < 0)
		return -1;
	if (d0 != -ENOSYS) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (d0 = %ld)\n", d0);
		return 0;
	}
#elif defined(IA64)
	if (upeek(tcp, PT_R10, &r10) < 0)
		return -1;
	if (upeek(tcp, PT_R8, &r8) < 0)
		return -1;
	if (ia32 && r8 != -ENOSYS) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (r8 = %ld)\n", r8);
		return 0;
	}
#elif defined(CRISV10) || defined(CRISV32)
	if (upeek(tcp, 4*PT_R10, &r10) < 0)
		return -1;
	if (r10 != -ENOSYS) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (r10 = %ld)\n", r10);
		return 0;
	}
#elif defined(MICROBLAZE)
	if (upeek(tcp, 3 * 4, &r3) < 0)
		return -1;
	if (r3 != -ENOSYS) {
		if (debug_flag)
			fprintf(stderr, "not a syscall entry (r3 = %ld)\n", r3);
		return 0;
	}
#endif
	return 1;
}

static void
internal_fork(struct tcb *tcp)
{
#if defined S390 || defined S390X || defined CRISV10 || defined CRISV32
# define ARG_FLAGS	1
#else
# define ARG_FLAGS	0
#endif
#ifndef CLONE_UNTRACED
# define CLONE_UNTRACED	0x00800000
#endif
	if ((ptrace_setoptions
	    & (PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK))
	   == (PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK))
		return;

	if (!followfork)
		return;

	if (entering(tcp)) {
		/*
		 * We won't see the new child if clone is called with
		 * CLONE_UNTRACED, so we keep the same logic with that option
		 * and don't trace it.
		 */
		if ((sysent[tcp->scno].sys_func == sys_clone) &&
		    (tcp->u_arg[ARG_FLAGS] & CLONE_UNTRACED))
			return;
		setbpt(tcp);
	} else {
		if (tcp->flags & TCB_BPTSET)
			clearbpt(tcp);
	}
}

#if defined(TCB_WAITEXECVE)
static void
internal_exec(struct tcb *tcp)
{
	/* Maybe we have post-execve SIGTRAP suppressed? */
	if (ptrace_setoptions & PTRACE_O_TRACEEXEC)
		return; /* yes, no need to do anything */

	if (exiting(tcp) && syserror(tcp))
		/* Error in execve, no post-execve SIGTRAP expected */
		tcp->flags &= ~TCB_WAITEXECVE;
	else
		tcp->flags |= TCB_WAITEXECVE;
}
#endif

static void
internal_syscall(struct tcb *tcp)
{
	/*
	 * We must always trace a few critical system calls in order to
	 * correctly support following forks in the presence of tracing
	 * qualifiers.
	 */
	int (*func)();

	if (!SCNO_IN_RANGE(tcp->scno))
		return;

	func = sysent[tcp->scno].sys_func;

	if (   sys_fork == func
	    || sys_vfork == func
	    || sys_clone == func
	   ) {
		internal_fork(tcp);
		return;
	}

#if defined(TCB_WAITEXECVE)
	if (   sys_execve == func
# if defined(SPARC) || defined(SPARC64)
	    || sys_execv == func
# endif
	   ) {
		internal_exec(tcp);
		return;
	}
#endif
}

/* Return -1 on error or 1 on success (never 0!) */
static int
get_syscall_args(struct tcb *tcp)
{
	int i, nargs;

	if (SCNO_IN_RANGE(tcp->scno))
		nargs = tcp->u_nargs = sysent[tcp->scno].nargs;
	else
		nargs = tcp->u_nargs = MAX_ARGS;

#if defined(S390) || defined(S390X)
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, i==0 ? PT_ORIGGPR2 : PT_GPR2 + i*sizeof(long), &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(ALPHA)
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, REG_A0+i, &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(IA64)
	if (!ia32) {
		unsigned long *out0, cfm, sof, sol;
		long rbs_end;
		/* be backwards compatible with kernel < 2.4.4... */
#		ifndef PT_RBS_END
#		  define PT_RBS_END	PT_AR_BSP
#		endif

		if (upeek(tcp, PT_RBS_END, &rbs_end) < 0)
			return -1;
		if (upeek(tcp, PT_CFM, (long *) &cfm) < 0)
			return -1;

		sof = (cfm >> 0) & 0x7f;
		sol = (cfm >> 7) & 0x7f;
		out0 = ia64_rse_skip_regs((unsigned long *) rbs_end, -sof + sol);

		for (i = 0; i < nargs; ++i) {
			if (umoven(tcp, (unsigned long) ia64_rse_skip_regs(out0, i),
				   sizeof(long), (char *) &tcp->u_arg[i]) < 0)
				return -1;
		}
	} else {
		static const int argreg[MAX_ARGS] = { PT_R11 /* EBX = out0 */,
						      PT_R9  /* ECX = out1 */,
						      PT_R10 /* EDX = out2 */,
						      PT_R14 /* ESI = out3 */,
						      PT_R15 /* EDI = out4 */,
						      PT_R13 /* EBP = out5 */};

		for (i = 0; i < nargs; ++i) {
			if (upeek(tcp, argreg[i], &tcp->u_arg[i]) < 0)
				return -1;
			/* truncate away IVE sign-extension */
			tcp->u_arg[i] &= 0xffffffff;
		}
	}
#elif defined(LINUX_MIPSN32) || defined(LINUX_MIPSN64)
	/* N32 and N64 both use up to six registers.  */
	unsigned long long regs[38];

	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long) &regs) < 0)
		return -1;

	for (i = 0; i < nargs; ++i) {
		tcp->u_arg[i] = regs[REG_A0 + i];
# if defined(LINUX_MIPSN32)
		tcp->ext_arg[i] = regs[REG_A0 + i];
# endif
	}
#elif defined(MIPS)
	if (nargs > 4) {
		long sp;

		if (upeek(tcp, REG_SP, &sp) < 0)
			return -1;
		for (i = 0; i < 4; ++i)
			if (upeek(tcp, REG_A0 + i, &tcp->u_arg[i]) < 0)
				return -1;
		umoven(tcp, sp + 16, (nargs - 4) * sizeof(tcp->u_arg[0]),
		       (char *)(tcp->u_arg + 4));
	} else {
		for (i = 0; i < nargs; ++i)
			if (upeek(tcp, REG_A0 + i, &tcp->u_arg[i]) < 0)
				return -1;
	}
#elif defined(POWERPC)
# ifndef PT_ORIG_R3
#  define PT_ORIG_R3 34
# endif
	for (i = 0; i < nargs; ++i) {
		if (upeek(tcp, (i==0) ?
			(sizeof(unsigned long) * PT_ORIG_R3) :
			((i+PT_R3) * sizeof(unsigned long)),
				&tcp->u_arg[i]) < 0)
			return -1;
	}
#elif defined(SPARC) || defined(SPARC64)
	for (i = 0; i < nargs; ++i)
		tcp->u_arg[i] = regs.u_regs[U_REG_O0 + i];
#elif defined(HPPA)
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, PT_GR26-4*i, &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(ARM)
	for (i = 0; i < nargs; ++i)
		tcp->u_arg[i] = regs.uregs[i];
#elif defined(AVR32)
	(void)i;
	(void)nargs;
	tcp->u_arg[0] = regs.r12;
	tcp->u_arg[1] = regs.r11;
	tcp->u_arg[2] = regs.r10;
	tcp->u_arg[3] = regs.r9;
	tcp->u_arg[4] = regs.r5;
	tcp->u_arg[5] = regs.r3;
#elif defined(BFIN)
	static const int argreg[MAX_ARGS] = { PT_R0, PT_R1, PT_R2, PT_R3, PT_R4, PT_R5 };

	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, argreg[i], &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(SH)
	static const int syscall_regs[MAX_ARGS] = {
		4 * (REG_REG0+4), 4 * (REG_REG0+5), 4 * (REG_REG0+6),
		4 * (REG_REG0+7), 4 * (REG_REG0  ), 4 * (REG_REG0+1)
	};

	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, syscall_regs[i], &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(SH64)
	int i;
	/* Registers used by SH5 Linux system calls for parameters */
	static const int syscall_regs[MAX_ARGS] = { 2, 3, 4, 5, 6, 7 };

	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, REG_GENERAL(syscall_regs[i]), &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(X86_64) || defined(X32)
	(void)i;
	(void)nargs;
	if (current_personality != 1) { /* x86-64 or x32 ABI */
		tcp->u_arg[0] = x86_64_regs.rdi;
		tcp->u_arg[1] = x86_64_regs.rsi;
		tcp->u_arg[2] = x86_64_regs.rdx;
		tcp->u_arg[3] = x86_64_regs.r10;
		tcp->u_arg[4] = x86_64_regs.r8;
		tcp->u_arg[5] = x86_64_regs.r9;
#  ifdef X32
		tcp->ext_arg[0] = x86_64_regs.rdi;
		tcp->ext_arg[1] = x86_64_regs.rsi;
		tcp->ext_arg[2] = x86_64_regs.rdx;
		tcp->ext_arg[3] = x86_64_regs.r10;
		tcp->ext_arg[4] = x86_64_regs.r8;
		tcp->ext_arg[5] = x86_64_regs.r9;
#  endif
	} else { /* i386 ABI */
		/* Sign-extend lower 32 bits */
		tcp->u_arg[0] = (long)(int)x86_64_regs.rbx;
		tcp->u_arg[1] = (long)(int)x86_64_regs.rcx;
		tcp->u_arg[2] = (long)(int)x86_64_regs.rdx;
		tcp->u_arg[3] = (long)(int)x86_64_regs.rsi;
		tcp->u_arg[4] = (long)(int)x86_64_regs.rdi;
		tcp->u_arg[5] = (long)(int)x86_64_regs.rbp;
	}
#elif defined(MICROBLAZE)
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, (5 + i) * 4, &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(CRISV10) || defined(CRISV32)
	static const int crisregs[MAX_ARGS] = {
		4*PT_ORIG_R10, 4*PT_R11, 4*PT_R12,
		4*PT_R13     , 4*PT_MOF, 4*PT_SRP
	};

	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, crisregs[i], &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(TILE)
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, PTREGS_OFFSET_REG(i), &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(M68K)
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, (i < 5 ? i : i + 2)*4, &tcp->u_arg[i]) < 0)
			return -1;
#elif defined(I386)
	(void)i;
	(void)nargs;
	tcp->u_arg[0] = i386_regs.ebx;
	tcp->u_arg[1] = i386_regs.ecx;
	tcp->u_arg[2] = i386_regs.edx;
	tcp->u_arg[3] = i386_regs.esi;
	tcp->u_arg[4] = i386_regs.edi;
	tcp->u_arg[5] = i386_regs.ebp;
#else /* Other architecture (32bits specific) */
	for (i = 0; i < nargs; ++i)
		if (upeek(tcp, i*4, &tcp->u_arg[i]) < 0)
			return -1;
#endif
	return 1;
}

static int
trace_syscall_entering(struct tcb *tcp)
{
	int res, scno_good;

#if defined TCB_WAITEXECVE
	if (tcp->flags & TCB_WAITEXECVE) {
		/* This is the post-execve SIGTRAP. */
		tcp->flags &= ~TCB_WAITEXECVE;
		return 0;
	}
#endif

	scno_good = res = get_scno(tcp);
	if (res == 0)
		return res;
	if (res == 1) {
		res = syscall_fixup_on_sysenter(tcp);
		if (res == 0)
			return res;
		if (res == 1)
			res = get_syscall_args(tcp);
	}

	if (res != 1) {
		printleader(tcp);
		if (scno_good != 1)
			tprints("????" /* anti-trigraph gap */ "(");
		else if (!SCNO_IN_RANGE(tcp->scno))
			tprintf("syscall_%lu(", tcp->scno);
		else
			tprintf("%s(", sysent[tcp->scno].sys_name);
		/*
		 * " <unavailable>" will be added later by the code which
		 * detects ptrace errors.
		 */
		goto ret;
	}

#if defined(SYS_socket_subcall) || defined(SYS_ipc_subcall)
	while (SCNO_IN_RANGE(tcp->scno)) {
# ifdef SYS_socket_subcall
		if (sysent[tcp->scno].sys_func == sys_socketcall) {
			decode_socket_subcall(tcp);
			break;
		}
# endif
# ifdef SYS_ipc_subcall
		if (sysent[tcp->scno].sys_func == sys_ipc) {
			decode_ipc_subcall(tcp);
			break;
		}
# endif
		break;
	}
#endif /* SYS_socket_subcall || SYS_ipc_subcall */

	internal_syscall(tcp);

	if ((SCNO_IN_RANGE(tcp->scno) &&
	     !(qual_flags[tcp->scno] & QUAL_TRACE)) ||
	    (tracing_paths && !pathtrace_match(tcp))) {
		tcp->flags |= TCB_INSYSCALL | TCB_FILTERED;
		return 0;
	}

	tcp->flags &= ~TCB_FILTERED;

	if (cflag == CFLAG_ONLY_STATS) {
		res = 0;
		goto ret;
	}

	printleader(tcp);
	if (!SCNO_IN_RANGE(tcp->scno))
		tprintf("syscall_%lu(", tcp->scno);
	else
		tprintf("%s(", sysent[tcp->scno].sys_name);
	if (!SCNO_IN_RANGE(tcp->scno) ||
	    ((qual_flags[tcp->scno] & QUAL_RAW) &&
	     sysent[tcp->scno].sys_func != sys_exit))
		res = printargs(tcp);
	else
		res = (*sysent[tcp->scno].sys_func)(tcp);

	if (fflush(tcp->outf) == EOF)
		return -1;
 ret:
	tcp->flags |= TCB_INSYSCALL;
	/* Measure the entrance time as late as possible to avoid errors. */
	if (Tflag || cflag)
		gettimeofday(&tcp->etime, NULL);
	return res;
}

/* Returns:
 * 1: ok, continue in trace_syscall_exiting().
 * -1: error, trace_syscall_exiting() should print error indicator
 *    ("????" etc) and bail out.
 */
static int
get_syscall_result(struct tcb *tcp)
{
#if defined(S390) || defined(S390X)
	if (upeek(tcp, PT_GPR2, &gpr2) < 0)
		return -1;
#elif defined(POWERPC)
# define SO_MASK 0x10000000
	{
		long flags;
		if (upeek(tcp, sizeof(unsigned long)*PT_CCR, &flags) < 0)
			return -1;
		if (upeek(tcp, sizeof(unsigned long)*PT_R3, &ppc_result) < 0)
			return -1;
		if (flags & SO_MASK)
			ppc_result = -ppc_result;
	}
#elif defined(AVR32)
	/* Read complete register set in one go. */
	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, &regs) < 0)
		return -1;
#elif defined(BFIN)
	if (upeek(tcp, PT_R0, &r0) < 0)
		return -1;
#elif defined(I386)
	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long) &i386_regs) < 0)
		return -1;
#elif defined(X86_64) || defined(X32)
	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long) &x86_64_regs) < 0)
		return -1;
#elif defined(IA64)
#	define IA64_PSR_IS	((long)1 << 34)
	if (upeek(tcp, PT_CR_IPSR, &psr) >= 0)
		ia32 = (psr & IA64_PSR_IS) != 0;
	if (upeek(tcp, PT_R8, &r8) < 0)
		return -1;
	if (upeek(tcp, PT_R10, &r10) < 0)
		return -1;
#elif defined(ARM)
	/* Read complete register set in one go. */
	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (void *)&regs) == -1)
		return -1;
#elif defined(M68K)
	if (upeek(tcp, 4*PT_D0, &d0) < 0)
		return -1;
#elif defined(LINUX_MIPSN32)
	unsigned long long regs[38];

	if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long) &regs) < 0)
		return -1;
	a3 = regs[REG_A3];
	r2 = regs[REG_V0];
#elif defined(MIPS)
	if (upeek(tcp, REG_A3, &a3) < 0)
		return -1;
	if (upeek(tcp, REG_V0, &r2) < 0)
		return -1;
#elif defined(ALPHA)
	if (upeek(tcp, REG_A3, &a3) < 0)
		return -1;
	if (upeek(tcp, REG_R0, &r0) < 0)
		return -1;
#elif defined(SPARC) || defined(SPARC64)
	/* Everything we need is in the current register set. */
	if (ptrace(PTRACE_GETREGS, tcp->pid, (char *)&regs, 0) < 0)
		return -1;
#elif defined(HPPA)
	if (upeek(tcp, PT_GR28, &r28) < 0)
		return -1;
#elif defined(SH)
#elif defined(SH64)
#elif defined(CRISV10) || defined(CRISV32)
	if (upeek(tcp, 4*PT_R10, &r10) < 0)
		return -1;
#elif defined(TILE)
#elif defined(MICROBLAZE)
	if (upeek(tcp, 3 * 4, &r3) < 0)
		return -1;
#endif

#if defined(SH)
	/* new syscall ABI returns result in R0 */
	if (upeek(tcp, 4*REG_REG0, (long *)&r0) < 0)
		return -1;
#elif defined(SH64)
	/* ABI defines result returned in r9 */
	if (upeek(tcp, REG_GENERAL(9), (long *)&r9) < 0)
		return -1;
#endif

	return 1;
}

/* Called at each syscall exit */
static void
syscall_fixup_on_sysexit(struct tcb *tcp)
{
#if defined(S390) || defined(S390X)
	if (syscall_mode != -ENOSYS)
		syscall_mode = tcp->scno;
	if ((tcp->flags & TCB_WAITEXECVE)
		 && (gpr2 == -ENOSYS || gpr2 == tcp->scno)) {
		/*
		 * Return from execve.
		 * Fake a return value of zero.  We leave the TCB_WAITEXECVE
		 * flag set for the post-execve SIGTRAP to see and reset.
		 */
		gpr2 = 0;
	}
#endif
}

/*
 * Check the syscall return value register value for whether it is
 * a negated errno code indicating an error, or a success return value.
 */
static inline int
is_negated_errno(unsigned long int val)
{
	unsigned long int max = -(long int) nerrnos;
#if SUPPORTED_PERSONALITIES > 1
	if (current_wordsize < sizeof(val)) {
		val = (unsigned int) val;
		max = (unsigned int) max;
	}
#endif
	return val > max;
}

/* Returns:
 * 1: ok, continue in trace_syscall_exiting().
 * -1: error, trace_syscall_exiting() should print error indicator
 *    ("????" etc) and bail out.
 */
static int
get_error(struct tcb *tcp)
{
	int u_error = 0;
	int check_errno = 1;
	if (SCNO_IN_RANGE(tcp->scno) &&
	    sysent[tcp->scno].sys_flags & SYSCALL_NEVER_FAILS) {
		check_errno = 0;
	}
#if defined(S390) || defined(S390X)
	if (check_errno && is_negated_errno(gpr2)) {
		tcp->u_rval = -1;
		u_error = -gpr2;
	}
	else {
		tcp->u_rval = gpr2;
	}
#elif defined(I386)
	if (check_errno && is_negated_errno(i386_regs.eax)) {
		tcp->u_rval = -1;
		u_error = -i386_regs.eax;
	}
	else {
		tcp->u_rval = i386_regs.eax;
	}
#elif defined(X86_64) || defined(X32)
	if (check_errno && is_negated_errno(x86_64_regs.rax)) {
		tcp->u_rval = -1;
		u_error = -x86_64_regs.rax;
	}
	else {
		tcp->u_rval = x86_64_regs.rax;
# if defined(X32)
		tcp->u_lrval = x86_64_regs.rax;
# endif
	}
#elif defined(IA64)
	if (ia32) {
		int err;

		err = (int)r8;
		if (check_errno && is_negated_errno(err)) {
			tcp->u_rval = -1;
			u_error = -err;
		}
		else {
			tcp->u_rval = err;
		}
	} else {
		if (check_errno && r10) {
			tcp->u_rval = -1;
			u_error = r8;
		} else {
			tcp->u_rval = r8;
		}
	}
#elif defined(MIPS)
	if (check_errno && a3) {
		tcp->u_rval = -1;
		u_error = r2;
	} else {
		tcp->u_rval = r2;
# if defined(LINUX_MIPSN32)
		tcp->u_lrval = r2;
# endif
	}
#elif defined(POWERPC)
	if (check_errno && is_negated_errno(ppc_result)) {
		tcp->u_rval = -1;
		u_error = -ppc_result;
	}
	else {
		tcp->u_rval = ppc_result;
	}
#elif defined(M68K)
	if (check_errno && is_negated_errno(d0)) {
		tcp->u_rval = -1;
		u_error = -d0;
	}
	else {
		tcp->u_rval = d0;
	}
#elif defined(ARM)
	if (check_errno && is_negated_errno(regs.ARM_r0)) {
		tcp->u_rval = -1;
		u_error = -regs.ARM_r0;
	}
	else {
		tcp->u_rval = regs.ARM_r0;
	}
#elif defined(AVR32)
	if (check_errno && regs.r12 && (unsigned) -regs.r12 < nerrnos) {
		tcp->u_rval = -1;
		u_error = -regs.r12;
	}
	else {
		tcp->u_rval = regs.r12;
	}
#elif defined(BFIN)
	if (check_errno && is_negated_errno(r0)) {
		tcp->u_rval = -1;
		u_error = -r0;
	} else {
		tcp->u_rval = r0;
	}
#elif defined(ALPHA)
	if (check_errno && a3) {
		tcp->u_rval = -1;
		u_error = r0;
	}
	else {
		tcp->u_rval = r0;
	}
#elif defined(SPARC)
	if (check_errno && regs.psr & PSR_C) {
		tcp->u_rval = -1;
		u_error = regs.u_regs[U_REG_O0];
	}
	else {
		tcp->u_rval = regs.u_regs[U_REG_O0];
	}
#elif defined(SPARC64)
	if (check_errno && regs.tstate & 0x1100000000UL) {
		tcp->u_rval = -1;
		u_error = regs.u_regs[U_REG_O0];
	}
	else {
		tcp->u_rval = regs.u_regs[U_REG_O0];
	}
#elif defined(HPPA)
	if (check_errno && is_negated_errno(r28)) {
		tcp->u_rval = -1;
		u_error = -r28;
	}
	else {
		tcp->u_rval = r28;
	}
#elif defined(SH)
	if (check_errno && is_negated_errno(r0)) {
		tcp->u_rval = -1;
		u_error = -r0;
	}
	else {
		tcp->u_rval = r0;
	}
#elif defined(SH64)
	if (check_errno && is_negated_errno(r9)) {
		tcp->u_rval = -1;
		u_error = -r9;
	}
	else {
		tcp->u_rval = r9;
	}
#elif defined(CRISV10) || defined(CRISV32)
	if (check_errno && r10 && (unsigned) -r10 < nerrnos) {
		tcp->u_rval = -1;
		u_error = -r10;
	}
	else {
		tcp->u_rval = r10;
	}
#elif defined(TILE)
	long rval;
	if (upeek(tcp, PTREGS_OFFSET_REG(0), &rval) < 0)
		return -1;
	if (check_errno && rval < 0 && rval > -nerrnos) {
		tcp->u_rval = -1;
		u_error = -rval;
	}
	else {
		tcp->u_rval = rval;
	}
#elif defined(MICROBLAZE)
	if (check_errno && is_negated_errno(r3)) {
		tcp->u_rval = -1;
		u_error = -r3;
	}
	else {
		tcp->u_rval = r3;
	}
#endif
	tcp->u_error = u_error;
	return 1;
}

static void
dumpio(struct tcb *tcp)
{
	if (syserror(tcp))
		return;
	if (tcp->u_arg[0] < 0 || tcp->u_arg[0] >= MAX_QUALS)
		return;
	if (!SCNO_IN_RANGE(tcp->scno))
		return;
	if (sysent[tcp->scno].sys_func == printargs)
		return;
	if (qual_flags[tcp->u_arg[0]] & QUAL_READ) {
		if (sysent[tcp->scno].sys_func == sys_read ||
		    sysent[tcp->scno].sys_func == sys_pread ||
		    sysent[tcp->scno].sys_func == sys_recv ||
		    sysent[tcp->scno].sys_func == sys_recvfrom)
			dumpstr(tcp, tcp->u_arg[1], tcp->u_rval);
		else if (sysent[tcp->scno].sys_func == sys_readv)
			dumpiov(tcp, tcp->u_arg[2], tcp->u_arg[1]);
		return;
	}
	if (qual_flags[tcp->u_arg[0]] & QUAL_WRITE) {
		if (sysent[tcp->scno].sys_func == sys_write ||
		    sysent[tcp->scno].sys_func == sys_pwrite ||
		    sysent[tcp->scno].sys_func == sys_send ||
		    sysent[tcp->scno].sys_func == sys_sendto)
			dumpstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		else if (sysent[tcp->scno].sys_func == sys_writev)
			dumpiov(tcp, tcp->u_arg[2], tcp->u_arg[1]);
		return;
	}
}

static int
trace_syscall_exiting(struct tcb *tcp)
{
	int sys_res;
	struct timeval tv;
	int res;
	long u_error;

	/* Measure the exit time as early as possible to avoid errors. */
	if (Tflag || cflag)
		gettimeofday(&tv, NULL);

#if SUPPORTED_PERSONALITIES > 1
	update_personality(tcp, tcp->currpers);
#endif
	res = get_syscall_result(tcp);
	if (res == 1) {
		syscall_fixup_on_sysexit(tcp); /* never fails */
		res = get_error(tcp); /* returns 1 or -1 */
		if (res == 1) {
			internal_syscall(tcp);
			if (filtered(tcp)) {
				goto ret;
			}
		}
	}

	if (cflag) {
		struct timeval t = tv;
		count_syscall(tcp, &t);
		if (cflag == CFLAG_ONLY_STATS) {
			goto ret;
		}
	}

	/* If not in -ff mode, and printing_tcp != tcp,
	 * then the log currently does not end with output
	 * of _our syscall entry_, but with something else.
	 * We need to say which syscall's return is this.
	 *
	 * Forced reprinting via TCB_REPRINT is used only by
	 * "strace -ff -oLOG test/threaded_execve" corner case.
	 * It's the only case when -ff mode needs reprinting.
	 */
	if ((followfork < 2 && printing_tcp != tcp) || (tcp->flags & TCB_REPRINT)) {
		tcp->flags &= ~TCB_REPRINT;
		printleader(tcp);
		if (!SCNO_IN_RANGE(tcp->scno))
			tprintf("<... syscall_%lu resumed> ", tcp->scno);
		else
			tprintf("<... %s resumed> ", sysent[tcp->scno].sys_name);
	}
	printing_tcp = tcp;

	if (res != 1) {
		/* There was error in one of prior ptrace ops */
		tprints(") ");
		tabto();
		tprints("= ? <unavailable>\n");
		line_ended();
		tcp->flags &= ~TCB_INSYSCALL;
		return res;
	}

	sys_res = 0;
	if (!SCNO_IN_RANGE(tcp->scno)
	    || (qual_flags[tcp->scno] & QUAL_RAW)) {
		/* sys_res = printargs(tcp); - but it's nop on sysexit */
	} else {
	/* FIXME: not_failing_only (IOW, option -z) is broken:
	 * failure of syscall is known only after syscall return.
	 * Thus we end up with something like this on, say, ENOENT:
	 *     open("doesnt_exist", O_RDONLY <unfinished ...>
	 *     {next syscall decode}
	 * whereas the intended result is that open(...) line
	 * is not shown at all.
	 */
		if (not_failing_only && tcp->u_error)
			goto ret;	/* ignore failed syscalls */
		sys_res = (*sysent[tcp->scno].sys_func)(tcp);
	}

	tprints(") ");
	tabto();
	u_error = tcp->u_error;
	if (!SCNO_IN_RANGE(tcp->scno) ||
	    qual_flags[tcp->scno] & QUAL_RAW) {
		if (u_error)
			tprintf("= -1 (errno %ld)", u_error);
		else
			tprintf("= %#lx", tcp->u_rval);
	}
	else if (!(sys_res & RVAL_NONE) && u_error) {
		switch (u_error) {
		/* Blocked signals do not interrupt any syscalls.
		 * In this case syscalls don't return ERESTARTfoo codes.
		 *
		 * Deadly signals set to SIG_DFL interrupt syscalls
		 * and kill the process regardless of which of the codes below
		 * is returned by the interrupted syscall.
		 * In some cases, kernel forces a kernel-generated deadly
		 * signal to be unblocked and set to SIG_DFL (and thus cause
		 * death) if it is blocked or SIG_IGNed: for example, SIGSEGV
		 * or SIGILL. (The alternative is to leave process spinning
		 * forever on the faulty instruction - not useful).
		 *
		 * SIG_IGNed signals and non-deadly signals set to SIG_DFL
		 * (for example, SIGCHLD, SIGWINCH) interrupt syscalls,
		 * but kernel will always restart them.
		 */
		case ERESTARTSYS:
			/* Most common type of signal-interrupted syscall exit code.
			 * The system call will be restarted with the same arguments
			 * if SA_RESTART is set; otherwise, it will fail with EINTR.
			 */
			tprints("= ? ERESTARTSYS (To be restarted if SA_RESTART is set)");
			break;
		case ERESTARTNOINTR:
			/* Rare. For example, fork() returns this if interrupted.
			 * SA_RESTART is ignored (assumed set): the restart is unconditional.
			 */
			tprints("= ? ERESTARTNOINTR (To be restarted)");
			break;
		case ERESTARTNOHAND:
			/* pause(), rt_sigsuspend() etc use this code.
			 * SA_RESTART is ignored (assumed not set):
			 * syscall won't restart (will return EINTR instead)
			 * even after signal with SA_RESTART set.
			 * However, after SIG_IGN or SIG_DFL signal it will.
			 */
			tprints("= ? ERESTARTNOHAND (Interrupted by signal)");
			break;
		case ERESTART_RESTARTBLOCK:
			/* Syscalls like nanosleep(), poll() which can't be
			 * restarted with their original arguments use this
			 * code. Kernel will execute restart_syscall() instead,
			 * which changes arguments before restarting syscall.
			 * SA_RESTART is ignored (assumed not set) similarly
			 * to ERESTARTNOHAND. (Kernel can't honor SA_RESTART
			 * since restart data is saved in "restart block"
			 * in task struct, and if signal handler uses a syscall
			 * which in turn saves another such restart block,
			 * old data is lost and restart becomes impossible)
			 */
			tprints("= ? ERESTART_RESTARTBLOCK (Interrupted by signal)");
			break;
		default:
			if (u_error < 0)
				tprintf("= -1 E??? (errno %ld)", u_error);
			else if (u_error < nerrnos)
				tprintf("= -1 %s (%s)", errnoent[u_error],
					strerror(u_error));
			else
				tprintf("= -1 ERRNO_%ld (%s)", u_error,
					strerror(u_error));
			break;
		}
		if ((sys_res & RVAL_STR) && tcp->auxstr)
			tprintf(" (%s)", tcp->auxstr);
	}
	else {
		if (sys_res & RVAL_NONE)
			tprints("= ?");
		else {
			switch (sys_res & RVAL_MASK) {
			case RVAL_HEX:
				tprintf("= %#lx", tcp->u_rval);
				break;
			case RVAL_OCTAL:
				tprintf("= %#lo", tcp->u_rval);
				break;
			case RVAL_UDECIMAL:
				tprintf("= %lu", tcp->u_rval);
				break;
			case RVAL_DECIMAL:
				tprintf("= %ld", tcp->u_rval);
				break;
#if defined(LINUX_MIPSN32) || defined(X32)
			/*
			case RVAL_LHEX:
				tprintf("= %#llx", tcp->u_lrval);
				break;
			case RVAL_LOCTAL:
				tprintf("= %#llo", tcp->u_lrval);
				break;
			*/
			case RVAL_LUDECIMAL:
				tprintf("= %llu", tcp->u_lrval);
				break;
			/*
			case RVAL_LDECIMAL:
				tprintf("= %lld", tcp->u_lrval);
				break;
			*/
#endif
			default:
				fprintf(stderr,
					"invalid rval format\n");
				break;
			}
		}
		if ((sys_res & RVAL_STR) && tcp->auxstr)
			tprintf(" (%s)", tcp->auxstr);
	}
	if (Tflag) {
		tv_sub(&tv, &tv, &tcp->etime);
		tprintf(" <%ld.%06ld>",
			(long) tv.tv_sec, (long) tv.tv_usec);
	}
	tprints("\n");
	dumpio(tcp);
	line_ended();

 ret:
	tcp->flags &= ~TCB_INSYSCALL;
	return 0;
}

int
trace_syscall(struct tcb *tcp)
{
	return exiting(tcp) ?
		trace_syscall_exiting(tcp) : trace_syscall_entering(tcp);
}
