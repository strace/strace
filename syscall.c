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
 *
 *	$Id$
 */

#include "defs.h"

#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/param.h>

#if HAVE_ASM_REG_H
#ifdef SPARC
#  define fpq kernel_fpq
#  define fq kernel_fq
#  define fpu kernel_fpu
#endif
#include <asm/reg.h>
#ifdef SPARC
#  undef fpq
#  undef fq
#  undef fpu 
#endif
#endif

#ifdef HAVE_SYS_REG_H
#include <sys/reg.h>
#ifndef PTRACE_PEEKUSR
# define PTRACE_PEEKUSR PTRACE_PEEKUSER
#endif
#elif defined(HAVE_LINUX_PTRACE_H)
#undef PTRACE_SYSCALL
#include <linux/ptrace.h>
#endif

#if defined(LINUX) && defined(IA64)
# include <asm/ptrace_offsets.h>
# include <asm/rse.h>
#endif

#ifndef SYS_ERRLIST_DECLARED
extern int sys_nerr;
extern char *sys_errlist[];
#endif /* SYS_ERRLIST_DECLARED */

#define NR_SYSCALL_BASE 0
#ifdef LINUX
#ifndef ERESTARTSYS
#define ERESTARTSYS	512
#endif
#ifndef ERESTARTNOINTR
#define ERESTARTNOINTR	513
#endif
#ifndef ERESTARTNOHAND
#define ERESTARTNOHAND	514	/* restart if no handler.. */
#endif
#ifndef ENOIOCTLCMD
#define ENOIOCTLCMD	515	/* No ioctl command */
#endif
#ifndef NSIG
#define NSIG 32
#endif
#ifdef ARM
#undef NSIG
#define NSIG 32
#undef NR_SYSCALL_BASE
#define NR_SYSCALL_BASE __NR_SYSCALL_BASE
#endif
#endif /* LINUX */

#include "syscall.h"

/* Define these shorthand notations to simplify the syscallent files. */
#define TF TRACE_FILE
#define TI TRACE_IPC
#define TN TRACE_NETWORK
#define TP TRACE_PROCESS
#define TS TRACE_SIGNAL

struct sysent sysent0[] = {
#include "syscallent.h"
};
int nsyscalls0 = sizeof sysent0 / sizeof sysent0[0];

#if SUPPORTED_PERSONALITIES >= 2
struct sysent sysent1[] = {
#include "syscallent1.h"
};
int nsyscalls1 = sizeof sysent1 / sizeof sysent1[0];
#endif /* SUPPORTED_PERSONALITIES >= 2 */

#if SUPPORTED_PERSONALITIES >= 3
struct sysent sysent2[] = {
#include "syscallent2.h"
};
int nsyscalls2 = sizeof sysent2 / sizeof sysent2[0];
#endif /* SUPPORTED_PERSONALITIES >= 3 */

struct sysent *sysent;
int nsyscalls;

/* Now undef them since short defines cause wicked namespace pollution. */
#undef TF
#undef TI
#undef TN
#undef TP
#undef TS

char *errnoent0[] = {
#include "errnoent.h"
};
int nerrnos0 = sizeof errnoent0 / sizeof errnoent0[0];

#if SUPPORTED_PERSONALITIES >= 2
char *errnoent1[] = {
#include "errnoent1.h"
};
int nerrnos1 = sizeof errnoent1 / sizeof errnoent1[0];
#endif /* SUPPORTED_PERSONALITIES >= 2 */

#if SUPPORTED_PERSONALITIES >= 3
char *errnoent2[] = {
#include "errnoent2.h"
};
int nerrnos2 = sizeof errnoent2 / sizeof errnoent2[0];
#endif /* SUPPORTED_PERSONALITIES >= 3 */

char **errnoent;
int nerrnos;

int current_personality;

int
set_personality(personality)
int personality;
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
		break;

#if SUPPORTED_PERSONALITIES >= 2
	case 1:
		errnoent = errnoent1;
		nerrnos = nerrnos1;
		sysent = sysent1;
		nsyscalls = nsyscalls1;
		ioctlent = ioctlent1;
		nioctlents = nioctlents1;
		signalent = signalent1;
		nsignals = nsignals1;
		break;
#endif /* SUPPORTED_PERSONALITIES >= 2 */

#if SUPPORTED_PERSONALITIES >= 3
	case 2:
		errnoent = errnoent2;
		nerrnos = nerrnos2;
		sysent = sysent2;
		nsyscalls = nsyscalls2;
		ioctlent = ioctlent2;
		nioctlents = nioctlents2;
		signalent = signalent2;
		nsignals = nsignals2;
		break;
#endif /* SUPPORTED_PERSONALITIES >= 3 */

	default:
		return -1;
	}

	current_personality = personality;
	return 0;
}

int qual_flags[MAX_QUALS];

static int call_count[MAX_QUALS];
static int error_count[MAX_QUALS];
static struct timeval tv_count[MAX_QUALS];
static int sorted_count[MAX_QUALS];

static struct timeval shortest = { 1000000, 0 };

static int lookup_syscall(), lookup_signal(), lookup_fault(), lookup_desc();

static struct qual_options {
	int bitflag;
	char *option_name;
	int (*lookup)();
	char *argument_name;
} qual_options[] = {
	{ QUAL_TRACE,	"trace",	lookup_syscall,	"system call"	},
	{ QUAL_TRACE,	"t",		lookup_syscall,	"system call"	},
	{ QUAL_ABBREV,	"abbrev",	lookup_syscall,	"system call"	},
	{ QUAL_ABBREV,	"a",		lookup_syscall,	"system call"	},
	{ QUAL_VERBOSE,	"verbose",	lookup_syscall,	"system call"	},
	{ QUAL_VERBOSE,	"v",		lookup_syscall,	"system call"	},
	{ QUAL_RAW,	"raw",		lookup_syscall,	"system call"	},
	{ QUAL_RAW,	"x",		lookup_syscall,	"system call"	},
	{ QUAL_SIGNAL,	"signal",	lookup_signal,	"signal"	},
	{ QUAL_SIGNAL,	"signals",	lookup_signal,	"signal"	},
	{ QUAL_SIGNAL,	"s",		lookup_signal,	"signal"	},
	{ QUAL_FAULT,	"fault",	lookup_fault,	"fault"		},
	{ QUAL_FAULT,	"faults",	lookup_fault,	"fault"		},
	{ QUAL_FAULT,	"m",		lookup_fault,	"fault"		},
	{ QUAL_READ,	"read",		lookup_desc,	"descriptor"	},
	{ QUAL_READ,	"reads",	lookup_desc,	"descriptor"	},
	{ QUAL_READ,	"r",		lookup_desc,	"descriptor"	},
	{ QUAL_WRITE,	"write",	lookup_desc,	"descriptor"	},
	{ QUAL_WRITE,	"writes",	lookup_desc,	"descriptor"	},
	{ QUAL_WRITE,	"w",		lookup_desc,	"descriptor"	},
	{ 0,		NULL,		NULL,		NULL		},
};

static int
lookup_syscall(s)
char *s;
{
	int i;

	for (i = 0; i < nsyscalls; i++) {
		if (strcmp(s, sysent[i].sys_name) == 0)
			return i;
	}
	return -1;
}

static int
lookup_signal(s)
char *s;
{
	int i;
	char buf[32];

	if (s && *s && isdigit((unsigned char)*s))
		return atoi(s);
	strcpy(buf, s);
	s = buf;
	for (i = 0; s[i]; i++)
		s[i] = toupper((unsigned char)(s[i]));
	if (strncmp(s, "SIG", 3) == 0)
		s += 3;
	for (i = 0; i <= NSIG; i++) {
		if (strcmp(s, signame(i) + 3) == 0)
			return i;
	}
	return -1;
}

static int
lookup_fault(s)
char *s;
{
	return -1;
}

static int
lookup_desc(s)
char *s;
{
	if (s && *s && isdigit((unsigned char)*s))
		return atoi(s);
	return -1;
}

static int
lookup_class(s)
char *s;
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
	return -1;
}

void
qualify(s)
char *s;
{
	struct qual_options *opt;
	int not;
	char *p;
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
			if (not)
				qual_flags[i] &= ~opt->bitflag;
			else
				qual_flags[i] |= opt->bitflag;
		}
		return;
	}
	for (i = 0; i < MAX_QUALS; i++) {
		if (not)
			qual_flags[i] |= opt->bitflag;
		else
			qual_flags[i] &= ~opt->bitflag;
	}
	for (p = strtok(s, ","); p; p = strtok(NULL, ",")) {
		if (opt->bitflag == QUAL_TRACE && (n = lookup_class(p)) > 0) {
			for (i = 0; i < MAX_QUALS; i++) {
				if (sysent[i].sys_flags & n) {
					if (not)
						qual_flags[i] &= ~opt->bitflag;
					else
						qual_flags[i] |= opt->bitflag;
				}
			}
			continue;
		}
		if ((n = (*opt->lookup)(p)) < 0) {
			fprintf(stderr, "strace: invalid %s `%s'\n",
				opt->argument_name, p);
			exit(1);
		}
		if (not)
			qual_flags[n] &= ~opt->bitflag;
		else
			qual_flags[n] |= opt->bitflag;
	}
	return;
}

static void
dumpio(tcp)
struct tcb *tcp;
{
	if (syserror(tcp))
		return;
	if (tcp->u_arg[0] < 0 || tcp->u_arg[0] >= MAX_QUALS)
		return;
	switch (tcp->scno + NR_SYSCALL_BASE) {
	case SYS_read:
#ifdef SYS_recv
	case SYS_recv:
#endif
#ifdef SYS_recvfrom
	case SYS_recvfrom:
#endif
		if (qual_flags[tcp->u_arg[0]] & QUAL_READ)
			dumpstr(tcp, tcp->u_arg[1], tcp->u_rval);
		break;
	case SYS_write:
#ifdef SYS_send
	case SYS_send:
#endif
#ifdef SYS_sendto
	case SYS_sendto:
#endif
		if (qual_flags[tcp->u_arg[0]] & QUAL_WRITE)
			dumpstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		break;
#ifdef SYS_readv
        case SYS_readv:
                if (qual_flags[tcp->u_arg[0]] & QUAL_READ)
                        dumpiov(tcp, tcp->u_arg[2], tcp->u_arg[1]);
                break;
#endif
#ifdef SYS_writev
        case SYS_writev:

                if (qual_flags[tcp->u_arg[0]] & QUAL_WRITE)
                        dumpiov(tcp, tcp->u_arg[2], tcp->u_arg[1]);
                break;
#endif
	}
}

#ifndef FREEBSD
enum subcall_style { shift_style, deref_style, mask_style, door_style };
#else /* FREEBSD */
enum subcall_style { shift_style, deref_style, mask_style, door_style, table_style };

struct subcall {
  int call;
  int nsubcalls;
  int subcalls[5];
};

const struct subcall subcalls_table[] = {
  { SYS_shmsys, 5, { SYS_shmat, SYS_shmctl, SYS_shmdt, SYS_shmget, SYS_shmctl } },
#ifdef SYS_semconfig
  { SYS_semsys, 4, { SYS___semctl, SYS_semget, SYS_semop, SYS_semconfig } },
#else
  { SYS_semsys, 3, { SYS___semctl, SYS_semget, SYS_semop } },
#endif
  { SYS_msgsys, 4, { SYS_msgctl, SYS_msgget, SYS_msgsnd, SYS_msgrcv } },
};
#endif /* FREEBSD */

#if !(defined(LINUX) && ( defined(ALPHA) || defined(MIPS) ))

const int socket_map [] = {
	       /* SYS_SOCKET      */ 97,
	       /* SYS_BIND        */ 104,
	       /* SYS_CONNECT     */ 98,
	       /* SYS_LISTEN      */ 106,
	       /* SYS_ACCEPT      */ 99,
	       /* SYS_GETSOCKNAME */ 150,
	       /* SYS_GETPEERNAME */ 141,
	       /* SYS_SOCKETPAIR  */ 135,
	       /* SYS_SEND        */ 101,
	       /* SYS_RECV        */ 102,
	       /* SYS_SENDTO      */ 133,
	       /* SYS_RECVFROM    */ 125,
	       /* SYS_SHUTDOWN    */ 134,
	       /* SYS_SETSOCKOPT  */ 105,
	       /* SYS_GETSOCKOPT  */ 118,
	       /* SYS_SENDMSG     */ 114,
	       /* SYS_RECVMSG     */ 113
};

void
sparc_socket_decode (tcp)
struct tcb *tcp;
{
	volatile long addr;
	volatile int i, n;

	if (tcp->u_arg [0] < 1 || tcp->u_arg [0] > sizeof(socket_map)/sizeof(int)+1){
		return;
	}
	tcp->scno = socket_map [tcp->u_arg [0]-1];
	n = tcp->u_nargs = sysent [tcp->scno].nargs;
	addr = tcp->u_arg [1];
	for (i = 0; i < n; i++){
	        int arg;
		if (umoven (tcp, addr, sizeof (arg), (void *) &arg) < 0)
			arg = 0;
		tcp->u_arg [i] = arg;
		addr += sizeof (arg);
	}
}

void
decode_subcall(tcp, subcall, nsubcalls, style)
struct tcb *tcp;
int subcall;
int nsubcalls;
enum subcall_style style;
{
	long addr, mask, arg;
	int i;

	switch (style) {
	case shift_style:
		if (tcp->u_arg[0] < 0 || tcp->u_arg[0] >= nsubcalls)
			return;
		tcp->scno = subcall + tcp->u_arg[0];
		if (sysent[tcp->scno].nargs != -1)
			tcp->u_nargs = sysent[tcp->scno].nargs;
		else
			tcp->u_nargs--;
		for (i = 0; i < tcp->u_nargs; i++)
			tcp->u_arg[i] = tcp->u_arg[i + 1];
		break;
	case deref_style:
		if (tcp->u_arg[0] < 0 || tcp->u_arg[0] >= nsubcalls)
			return;
		tcp->scno = subcall + tcp->u_arg[0];
		addr = tcp->u_arg[1];
		for (i = 0; i < sysent[tcp->scno].nargs; i++) {
			if (umove(tcp, addr, &arg) < 0)
				arg = 0;
			tcp->u_arg[i] = arg;
			addr += sizeof(arg);
		}
		tcp->u_nargs = sysent[tcp->scno].nargs;
		break;
	case mask_style:
		mask = (tcp->u_arg[0] >> 8) & 0xff;
		for (i = 0; mask; i++)
			mask >>= 1;
		if (i >= nsubcalls)
			return;
		tcp->u_arg[0] &= 0xff;
		tcp->scno = subcall + i;
		if (sysent[tcp->scno].nargs != -1)
			tcp->u_nargs = sysent[tcp->scno].nargs;
		break;
	case door_style:
		/*
		 * Oh, yuck.  The call code is the *sixth* argument.
		 * (don't you mean the *last* argument? - JH)
		 */
		if (tcp->u_arg[5] < 0 || tcp->u_arg[5] >= nsubcalls)
			return;
		tcp->scno = subcall + tcp->u_arg[5];
		if (sysent[tcp->scno].nargs != -1)
			tcp->u_nargs = sysent[tcp->scno].nargs;
		else
			tcp->u_nargs--;
		break;
#ifdef FREEBSD
	case table_style:
		for (i = 0; i < sizeof(subcalls_table) / sizeof(struct subcall); i++)
			if (subcalls_table[i].call == tcp->scno) break;
		if (i < sizeof(subcalls_table) / sizeof(struct subcall) &&
		    tcp->u_arg[0] >= 0 && tcp->u_arg[0] < subcalls_table[i].nsubcalls) {
			tcp->scno = subcalls_table[i].subcalls[tcp->u_arg[0]];
			for (i = 0; i < tcp->u_nargs; i++)
				tcp->u_arg[i] = tcp->u_arg[i + 1];
		}
		break;
#endif /* FREEBSD */
	}
}
#endif

struct tcb *tcp_last = NULL;

static int
internal_syscall(tcp)
struct tcb *tcp;
{
	/*
	 * We must always trace a few critical system calls in order to
	 * correctly support following forks in the presence of tracing
	 * qualifiers.
	 */
	switch (tcp->scno + NR_SYSCALL_BASE) {
#ifdef SYS_fork
	case SYS_fork:
#endif
#ifdef SYS_vfork
	case SYS_vfork:
#endif
#ifdef SYS_fork1
	case SYS_fork1:
#endif
#ifdef SYS_forkall
	case SYS_forkall:
#endif
#ifdef SYS_rfork1
	case SYS_rfork1:
#endif
#ifdef SYS_rforkall
	case SYS_rforkall:
#endif
		internal_fork(tcp);
		break;
#ifdef SYS_clone
	case SYS_clone:
		internal_clone(tcp);
		break;
#endif
#ifdef SYS_clone2
	case SYS_clone2:
		internal_clone(tcp);
		break;
#endif
#ifdef SYS_execv
	case SYS_execv:
#endif
#ifdef SYS_execve
	case SYS_execve:
#endif
#ifdef SYS_rexecve
	case SYS_rexecve:
#endif
		internal_exec(tcp);
		break;

#ifdef SYS_wait
	case SYS_wait:
#endif
#ifdef SYS_wait4
	case SYS_wait4:
#endif
#ifdef SYS32_wait4
	case SYS32_wait4:
#endif
#ifdef SYS_waitpid
	case SYS_waitpid:
#endif
#ifdef SYS_waitsys
	case SYS_waitsys:
#endif
		internal_wait(tcp);
		break;

#ifdef SYS_exit
	case SYS_exit:
#endif
#ifdef SYS32_exit
	case SYS32_exit:
#endif
		internal_exit(tcp);
		break;
	}
	return 0;
}


#ifdef LINUX
#if defined (I386)
	static long eax;
#elif defined (IA64)
	long r8, r10, psr;
	long ia32 = 0;
#elif defined (POWERPC)
	static long result,flags;
#elif defined (M68K)
	static int d0;
#elif defined (ARM)
	static int r0;
#elif defined (ALPHA)
	static long r0;
	static long a3;
#elif defined (SPARC)
	static struct regs regs;
	static unsigned long trap;
#elif defined(MIPS)
	static long a3;
	static long r2;
#elif defined(S390) || defined(S390X)
	static long gpr2;
	static long pc;
#elif defined(HPPA)
	static long r28;
#elif defined(SH)
       static long r0;
#elif defined(X86_64)
       static long rax;
#endif 
#endif /* LINUX */
#ifdef FREEBSD
	struct reg regs;
#endif /* FREEBSD */	

int
get_scno(tcp)
struct tcb *tcp;
{
	long scno = 0;
	static int currpers=-1;
#ifndef USE_PROCFS
	int pid = tcp->pid;
#endif /* !PROCFS */	

#ifdef LINUX
#if defined(S390) || defined(S390X)
	if (upeek(pid,PT_PSWADDR,&pc) < 0)
		return -1;
	scno = ptrace(PTRACE_PEEKTEXT, pid, (char *)(pc-sizeof(long)),0);
	if (errno)
		return -1;
	scno&=0xFF;
#elif defined (POWERPC)
	if (upeek(pid, 4*PT_R0, &scno) < 0)
		return -1;
	if (!(tcp->flags & TCB_INSYSCALL)) {
		/* Check if we return from execve. */
		if (scno == 0 && (tcp->flags & TCB_WAITEXECVE)) {
			tcp->flags &= ~TCB_WAITEXECVE;
			return 0;
		}
	}
#elif defined (I386)
	if (upeek(pid, 4*ORIG_EAX, &scno) < 0)
		return -1;
#elif defined (X86_64)
	if (upeek(pid, 8*ORIG_RAX, &scno) < 0)
		return -1;

	if (!(tcp->flags & TCB_INSYSCALL)) { 
		long val;

		/* Check CS register value. On x86-64 linux it is:
		 * 	0x33	for long mode (64 bit)
		 * 	0x23	for compatibility mode (32 bit)
		 * It takes only one ptrace and thus doesn't need 
		 * to be cached.
		 */
		if (upeek(pid, 8*CS, &val) < 0)
			return -1;
		switch(val)
		{
			case 0x23: currpers = 1; break;
			case 0x33: currpers = 0; break;
			default:
				fprintf(stderr, "Unknown value CS=0x%02X while "
					 "detecting personality of process "
					 "PID=%d\n", (int)val, pid);
				currpers = current_personality;
				break;
		}
#if 0
		/* This version analyzes the opcode of a syscall instruction.
		 * (int 0x80 on i386 vs. syscall on x86-64)
		 * It works, but is too complicated.
		 */
		unsigned long val, rip, i;

		if(upeek(pid, 8*RIP, &rip)<0)
			perror("upeek(RIP)");
		
		/* sizeof(syscall) == sizeof(int 0x80) == 2 */
		rip-=2;
		errno = 0;

		call = ptrace(PTRACE_PEEKTEXT,pid,(char *)rip,0); 
		if (errno) 
			printf("ptrace_peektext failed: %s\n", 
					strerror(errno));
		switch (call & 0xffff)
		{
			/* x86-64: syscall = 0x0f 0x05 */
			case 0x050f: currpers = 0; break;
			/* i386: int 0x80 = 0xcd 0x80 */
			case 0x80cd: currpers = 1; break;
			default:
				currpers = current_personality;
				fprintf(stderr, 
					"Unknown syscall opcode (0x%04X) while "
					"detecting personality of process "
					"PID=%d\n", (int)call, pid);
				break;
		}
#endif
		if(currpers != current_personality)
		{
			char *names[]={"64 bit", "32 bit"};
			set_personality(currpers);
			printf("[ Process PID=%d runs in %s mode. ]\n", 
					pid, names[current_personality]);
		}
	} 
#elif defined(IA64)
#	define IA64_PSR_IS	((long)1 << 34)
	if (upeek (pid, PT_CR_IPSR, &psr) >= 0)
		ia32 = (psr & IA64_PSR_IS) != 0;
	if (!(tcp->flags & TCB_INSYSCALL)) {
		if (ia32) {
			if (upeek(pid, PT_R1, &scno) < 0)	/* orig eax */
				return -1;
			/* Check if we return from execve. */
		} else {
			if (upeek (pid, PT_R15, &scno) < 0)
				return -1;
		}
	} else {
		/* syscall in progress */
		if (upeek (pid, PT_R8, &r8) < 0)
			return -1;
		if (upeek (pid, PT_R10, &r10) < 0)
			return -1;
	}
	if (tcp->flags & TCB_WAITEXECVE) {
		tcp->flags &= ~TCB_WAITEXECVE;
		return 0;
	}

#elif defined (ARM)
	{ 
	    long pc;
	    upeek(pid, 4*15, &pc);
	    umoven(tcp, pc-4, 4, (char *)&scno);
	    scno &= 0x000fffff;
	}
#elif defined (M68K)
	if (upeek(pid, 4*PT_ORIG_D0, &scno) < 0)
		return -1;
#elif defined (MIPS)
	if (upeek(pid, REG_A3, &a3) < 0)
	  	return -1;

	if(!(tcp->flags & TCB_INSYSCALL)) {
	  	if (upeek(pid, REG_V0, &scno) < 0)
		  	return -1;

		if (scno < 0 || scno > nsyscalls) {
			if(a3 == 0 || a3 == -1) {
				if(debug)
					fprintf (stderr, "stray syscall exit: v0 = %ld\n", scno);
				return 0;
			}
		}
	} else {
	  	if (upeek(pid, REG_V0, &r2) < 0)
	    		return -1;
	}
#elif defined (ALPHA)
	if (upeek(pid, REG_A3, &a3) < 0)
		return -1;

	if (!(tcp->flags & TCB_INSYSCALL)) {
		if (upeek(pid, REG_R0, &scno) < 0)
			return -1;

		/* Check if we return from execve. */
		if (scno == 0 && tcp->flags & TCB_WAITEXECVE) {
			tcp->flags &= ~TCB_WAITEXECVE;
			return 0;
		}

		/*
		 * Do some sanity checks to figure out if it's
		 * really a syscall entry
		 */
		if (scno < 0 || scno > nsyscalls) {
			if (a3 == 0 || a3 == -1) {
				if (debug)
					fprintf (stderr, "stray syscall exit: r0 = %ld\n", scno);
				return 0;
			}
		}
	}
	else {
		if (upeek(pid, REG_R0, &r0) < 0)
			return -1;
	}
#elif defined (SPARC)
	/* Everything we need is in the current register set. */
	if (ptrace(PTRACE_GETREGS,pid,(char *)&regs,0) < 0)
		return -1;

        /* If we are entering, then disassemble the syscall trap. */
	if (!(tcp->flags & TCB_INSYSCALL)) {
		/* Retrieve the syscall trap instruction. */
		errno = 0;
		trap = ptrace(PTRACE_PEEKTEXT,pid,(char *)regs.r_pc,0);
		if (errno)
			return -1;

		/* Disassemble the trap to see what personality to use. */
		switch (trap) {
		case 0x91d02010:
			/* Linux/SPARC syscall trap. */
			set_personality(0);
			break;
		case 0x91d0206d:
			/* Linux/SPARC64 syscall trap. */
			fprintf(stderr,"syscall: Linux/SPARC64 not supported yet\n");
			return -1;
		case 0x91d02000:
			/* SunOS syscall trap. (pers 1) */
			fprintf(stderr,"syscall: SunOS no support\n");
			return -1;
		case 0x91d02008:
			/* Solaris 2.x syscall trap. (per 2) */
			set_personality(1);
			break; 
		case 0x91d02009:
			/* NetBSD/FreeBSD syscall trap. */
			fprintf(stderr,"syscall: NetBSD/FreeBSD not supported\n");
			return -1;
		case 0x91d02027:
			/* Solaris 2.x gettimeofday */
			set_personality(1);
			break;
		default:
			/* Unknown syscall trap. */
			if(tcp->flags & TCB_WAITEXECVE) {
				tcp->flags &= ~TCB_WAITEXECVE;
				return 0;
			}
			fprintf(stderr,"syscall: unknown syscall trap %08x %08x\n", trap, regs.r_pc);
			return -1;
		}

		/* Extract the system call number from the registers. */
		if (trap == 0x91d02027)
			scno = 156;
		else
			scno = regs.r_g1;
		if (scno == 0) {
			scno = regs.r_o0;
			memmove (&regs.r_o0, &regs.r_o1, 7*sizeof(regs.r_o0));
		}
	}
#elif defined(HPPA)
	if (upeek(pid, PT_GR20, &scno) < 0)
		return -1;
	if (!(tcp->flags & TCB_INSYSCALL)) {
		/* Check if we return from execve. */
		if ((tcp->flags & TCB_WAITEXECVE)) {
			tcp->flags &= ~TCB_WAITEXECVE;
			return 0;
		}
	}
#elif defined(SH)
       /*
        * In the new syscall ABI, the system call number is in R3.
        */
       if (upeek(pid, 4*(REG_REG0+3), &scno) < 0)
               return -1;

       if (scno < 0) {
           /* Odd as it may seem, a glibc bug has been known to cause
              glibc to issue bogus negative syscall numbers.  So for
              our purposes, make strace print what it *should* have been */
           long correct_scno = (scno & 0xff);
           if (debug)
               fprintf(stderr,
                   "Detected glibc bug: bogus system call number = %ld, "
		   "correcting to %ld\n",
                   scno,
                   correct_scno);
           scno = correct_scno;
       }


       if (!(tcp->flags & TCB_INSYSCALL)) {
               /* Check if we return from execve. */
               if (scno == 0 && tcp->flags & TCB_WAITEXECVE) {
                       tcp->flags &= ~TCB_WAITEXECVE;
                       return 0;
               }
       }
#endif /* SH */
#endif /* LINUX */
#ifdef SUNOS4
	if (upeek(pid, uoff(u_arg[7]), &scno) < 0)
		return -1;
#elif defined(SH)
       /* new syscall ABI returns result in R0 */
       if (upeek(pid, 4*REG_REG0, (long *)&r0) < 0)
               return -1;
#endif
#ifdef USE_PROCFS
#ifdef HAVE_PR_SYSCALL
	scno = tcp->status.PR_SYSCALL;
#else /* !HAVE_PR_SYSCALL */
#ifndef FREEBSD
	scno = tcp->status.PR_WHAT;
#else /* FREEBSD */
	if (pread(tcp->pfd_reg, &regs, sizeof(regs), 0) < 0) {
	        perror("pread");
                return -1;
        }
	switch (regs.r_eax) {
	case SYS_syscall:
	case SYS___syscall:
    	        pread(tcp->pfd, &scno, sizeof(scno), regs.r_esp + sizeof(int));
	        break;
	default:
	        scno = regs.r_eax;
	        break;
	}
#endif /* FREEBSD */
#endif /* !HAVE_PR_SYSCALL */
#endif /* USE_PROCFS */
	if (!(tcp->flags & TCB_INSYSCALL))
		tcp->scno = scno;
	return 1;
}


int
syscall_fixup(tcp)
struct tcb *tcp;
{
#ifndef USE_PROCFS
	int pid = tcp->pid;
#else /* USE_PROCFS */	
	int scno = tcp->scno;

	if (!(tcp->flags & TCB_INSYSCALL)) {
		if (tcp->status.PR_WHY != PR_SYSENTRY) {
			if (
			    scno == SYS_fork
#ifdef SYS_vfork
			    || scno == SYS_vfork
#endif /* SYS_vfork */
#ifdef SYS_fork1
			    || scno == SYS_fork1
#endif /* SYS_fork1 */
#ifdef SYS_forkall
			    || scno == SYS_forkall
#endif /* SYS_forkall */
#ifdef SYS_rfork1
			    || scno == SYS_rfork1
#endif /* SYS_fork1 */
#ifdef SYS_rforkall
			    || scno == SYS_rforkall
#endif /* SYS_rforkall */
			    ) {
				/* We are returning in the child, fake it. */
				tcp->status.PR_WHY = PR_SYSENTRY;
				trace_syscall(tcp);
				tcp->status.PR_WHY = PR_SYSEXIT;
			}
			else {
				fprintf(stderr, "syscall: missing entry\n");
				tcp->flags |= TCB_INSYSCALL;
			}
		}
	}
	else {
		if (tcp->status.PR_WHY != PR_SYSEXIT) {
			fprintf(stderr, "syscall: missing exit\n");
			tcp->flags &= ~TCB_INSYSCALL;
		}
	}
#endif /* USE_PROCFS */
#ifdef SUNOS4
	if (!(tcp->flags & TCB_INSYSCALL)) {
		if (scno == 0) {
			fprintf(stderr, "syscall: missing entry\n");
			tcp->flags |= TCB_INSYSCALL;
		}
	}
	else {
		if (scno != 0) {
			if (debug) {
				/*
				 * This happens when a signal handler
				 * for a signal which interrupted a
				 * a system call makes another system call.
				 */
				fprintf(stderr, "syscall: missing exit\n");
			}
			tcp->flags &= ~TCB_INSYSCALL;
		}
	}
#endif /* SUNOS4 */
#ifdef LINUX
#if defined (I386)
	if (upeek(pid, 4*EAX, &eax) < 0)
		return -1;
	if (eax != -ENOSYS && !(tcp->flags & TCB_INSYSCALL)) {
		if (debug)
			fprintf(stderr, "stray syscall exit: eax = %ld\n", eax);
		return 0;
	}
#elif defined (X86_64)
	if (upeek(pid, 8*RAX, &rax) < 0)
		return -1;
	if (rax != -ENOSYS && !(tcp->flags & TCB_INSYSCALL)) {
		if (debug)
			fprintf(stderr, "stray syscall exit: rax = %ld\n", rax);
		return 0;
	}
#elif defined (S390) || defined (S390X)
	if (upeek(pid, PT_GPR2, &gpr2) < 0)
		return -1;
	if (gpr2 != -ENOSYS && !(tcp->flags & TCB_INSYSCALL)) {
		if (debug)
			fprintf(stderr, "stray syscall exit: gpr2 = %ld\n", gpr2);
		return 0;
	}
#elif defined (POWERPC)
# define SO_MASK 0x10000000
	if (upeek(pid, 4*PT_CCR, &flags) < 0)
		return -1;
	if (upeek(pid, 4*PT_R3, &result) < 0)
		return -1;
	if (flags & SO_MASK)
		result = -result;
#elif defined (M68K)
	if (upeek(pid, 4*PT_D0, &d0) < 0)
		return -1;
	if (d0 != -ENOSYS && !(tcp->flags & TCB_INSYSCALL)) {
		if (debug)
			fprintf(stderr, "stray syscall exit: d0 = %ld\n", d0);
		return 0;
	}
#elif defined (ARM)
	if (upeek(pid, 4*0, (long *)&r0) < 0)
		return -1;
	if ( 0 && r0 != -ENOSYS && !(tcp->flags & TCB_INSYSCALL)) {
		if (debug)
			fprintf(stderr, "stray syscall exit: d0 = %ld\n", r0);
		return 0;
	}
#elif defined (HPPA)
	if (upeek(pid, PT_GR28, &r28) < 0)
		return -1;
#elif defined(IA64)
	if (upeek(pid, PT_R10, &r10) < 0)
		return -1;
	if (upeek(pid, PT_R8, &r8) < 0)
		return -1;
	if (ia32 && r8 != -ENOSYS && !(tcp->flags & TCB_INSYSCALL)) {
		if (debug)
			fprintf(stderr, "stray syscall exit: r8 = %ld\n", r8);
		return 0;
	}
#endif
#endif /* LINUX */
	return 1;
}

int
get_error(tcp)
struct tcb *tcp;
{
	int u_error = 0;
#ifdef LINUX
#if defined(S390) || defined(S390X)
		if (gpr2 && (unsigned) -gpr2 < nerrnos) {
			tcp->u_rval = -1;
			u_error = -gpr2;
		}
		else {
			tcp->u_rval = gpr2;
			u_error = 0;
		}
#else /* !S390 && !S390X */
#ifdef I386
		if (eax < 0 && -eax < nerrnos) {
			tcp->u_rval = -1;
			u_error = -eax;
		}
		else {
			tcp->u_rval = eax;
			u_error = 0;
		}
#else /* !I386 */
#ifdef X86_64
		if (rax < 0 && -rax < nerrnos) {
			tcp->u_rval = -1;
			u_error = -rax;
		}
		else {
			tcp->u_rval = rax;
			u_error = 0;
		}
#else
#ifdef IA64
		if (ia32) {
			int err;

			err = (int)r8;
			if (err < 0 && -err < nerrnos) {
				tcp->u_rval = -1;
				u_error = -err;
			}
			else {
				tcp->u_rval = err;
				u_error = 0;
			}
		} else {
			if (r10) {
				tcp->u_rval = -1;
				u_error = r8;
			} else {
				tcp->u_rval = r8;
				u_error = 0;
			}
		}
#else /* !IA64 */
#ifdef MIPS
		if (a3) {
		  	tcp->u_rval = -1;
			u_error = r2;
		} else {
		  	tcp->u_rval = r2;
			u_error = 0;
		}
#else
#ifdef POWERPC
		if (result && (unsigned) -result < nerrnos) {
			tcp->u_rval = -1;
			u_error = -result;
		}
		else {
			tcp->u_rval = result;
			u_error = 0;
		}
#else /* !POWERPC */
#ifdef M68K
		if (d0 && (unsigned) -d0 < nerrnos) {
			tcp->u_rval = -1;
			u_error = -d0;
		}
		else {
			tcp->u_rval = d0;
			u_error = 0;
		}
#else /* !M68K */
#ifdef ARM
		if (r0 && (unsigned) -r0 < nerrnos) {
			tcp->u_rval = -1;
			u_error = -r0;
		}
		else {
			tcp->u_rval = r0;
			u_error = 0;
		}
#else /* !ARM */
#ifdef ALPHA
		if (a3) {
			tcp->u_rval = -1;
			u_error = r0;
		}
		else {
			tcp->u_rval = r0;
			u_error = 0;
		}
#else /* !ALPHA */
#ifdef SPARC
		if (regs.r_psr & PSR_C) {
			tcp->u_rval = -1;
			u_error = regs.r_o0;
		}
		else {
			tcp->u_rval = regs.r_o0;
			u_error = 0;
		}
#else /* !SPARC */
#ifdef HPPA
		if (r28 && (unsigned) -r28 < nerrnos) {
			tcp->u_rval = -1;
			u_error = -r28;
		}
		else {
			tcp->u_rval = r28;
			u_error = 0;
		}
#else
#ifdef SH
               /* interpret R0 as return value or error number */
               if (r0 && (unsigned) -r0 < nerrnos) {
                       tcp->u_rval = -1;
                       u_error = -r0;
               }
               else {
                       tcp->u_rval = r0;
                       u_error = 0;
               }
#endif /* SH */
#endif /* HPPA */
#endif /* SPARC */
#endif /* ALPHA */
#endif /* ARM */
#endif /* M68K */
#endif /* POWERPC */
#endif /* MIPS */
#endif /* IA64 */
#endif /* X86_64 */
#endif /* I386 */
#endif /* S390 || S390X */
#endif /* LINUX */
#ifdef SUNOS4
		/* get error code from user struct */
		if (upeek(pid, uoff(u_error), &u_error) < 0)
			return -1;
		u_error >>= 24; /* u_error is a char */

		/* get system call return value */
		if (upeek(pid, uoff(u_rval1), &tcp->u_rval) < 0)
			return -1;
#endif /* SUNOS4 */
#ifdef SVR4
#ifdef SPARC
		/* Judicious guessing goes a long way. */
		if (tcp->status.pr_reg[R_PSR] & 0x100000) {
			tcp->u_rval = -1;
			u_error = tcp->status.pr_reg[R_O0];
		}
		else {
			tcp->u_rval = tcp->status.pr_reg[R_O0];
			u_error = 0;
		}
#endif /* SPARC */
#ifdef I386
		/* Wanna know how to kill an hour single-stepping? */
		if (tcp->status.PR_REG[EFL] & 0x1) {
			tcp->u_rval = -1;
			u_error = tcp->status.PR_REG[EAX];
		}
		else {
			tcp->u_rval = tcp->status.PR_REG[EAX];
#ifdef HAVE_LONG_LONG
			tcp->u_lrval =
				((unsigned long long) tcp->status.PR_REG[EDX] << 32) +
				tcp->status.PR_REG[EAX];
#endif
			u_error = 0;
		}
#endif /* I386 */
#ifdef X86_64
		/* Wanna know how to kill an hour single-stepping? */
		if (tcp->status.PR_REG[EFLAGS] & 0x1) {
			tcp->u_rval = -1;
			u_error = tcp->status.PR_REG[RAX];
		}
		else {
			tcp->u_rval = tcp->status.PR_REG[RAX];
			u_error = 0;
		}
#endif /* X86_64 */
#ifdef MIPS
		if (tcp->status.pr_reg[CTX_A3]) {
			tcp->u_rval = -1;
			u_error = tcp->status.pr_reg[CTX_V0];
		}
		else {
			tcp->u_rval = tcp->status.pr_reg[CTX_V0];
			u_error = 0;
		}
#endif /* MIPS */
#endif /* SVR4 */
#ifdef FREEBSD
		if (regs.r_eflags & PSL_C) {
 		        tcp->u_rval = -1;
		        u_error = regs.r_eax;
		} else {
		        tcp->u_rval = regs.r_eax;
			tcp->u_lrval =
			  ((unsigned long long) regs.r_edx << 32) +  regs.r_eax;
		        u_error = 0;
		}
#endif /* FREEBSD */	
	tcp->u_error = u_error;
	return 1;
}

int syscall_enter(tcp)
struct tcb *tcp;
{
#ifndef USE_PROCFS
	int pid = tcp->pid;
#endif /* !USE_PROCFS */	
#ifdef LINUX
#if defined(S390) || defined(S390X)
	{
		int i;
		if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
			tcp->u_nargs = sysent[tcp->scno].nargs;
		else 
     	        	tcp->u_nargs = MAX_ARGS;
		for (i = 0; i < tcp->u_nargs; i++) {
			if (upeek(pid,i==0 ? PT_ORIGGPR2:PT_GPR2+i*sizeof(long), &tcp->u_arg[i]) < 0)
				return -1;
		}
	}
#elif defined (ALPHA)
	{
		int i;
		if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
			tcp->u_nargs = sysent[tcp->scno].nargs;
		else 
     	        	tcp->u_nargs = MAX_ARGS;
		for (i = 0; i < tcp->u_nargs; i++) {
			/* WTA: if scno is out-of-bounds this will bomb. Add range-check
			 * for scno somewhere above here!
			 */
			if (upeek(pid, REG_A0+i, &tcp->u_arg[i]) < 0)
				return -1;
		}
	}
#elif defined (IA64)
	{
		if (!ia32) {
			unsigned long *out0, *rbs_end, cfm, sof, sol, i;
			/* be backwards compatible with kernel < 2.4.4... */
#			ifndef PT_RBS_END
#			  define PT_RBS_END	PT_AR_BSP
#			endif

			if (upeek(pid, PT_RBS_END, (long *) &rbs_end) < 0)
				return -1;
			if (upeek(pid, PT_CFM, (long *) &cfm) < 0)
				return -1;

			sof = (cfm >> 0) & 0x7f;
			sol = (cfm >> 7) & 0x7f;
			out0 = ia64_rse_skip_regs(rbs_end, -sof + sol);

			if (tcp->scno >= 0 && tcp->scno < nsyscalls
			    && sysent[tcp->scno].nargs != -1)
				tcp->u_nargs = sysent[tcp->scno].nargs;
			else
				tcp->u_nargs = MAX_ARGS;
			for (i = 0; i < tcp->u_nargs; ++i) {
				if (umoven(tcp, (unsigned long) ia64_rse_skip_regs(out0, i),
					   sizeof(long), (char *) &tcp->u_arg[i]) < 0)
					return -1;
			}
		} else {
			int i;

			if (/* EBX = out0 */
			    upeek(pid, PT_R11, (long *) &tcp->u_arg[0]) < 0
			    /* ECX = out1 */
			    || upeek(pid, PT_R9,  (long *) &tcp->u_arg[1]) < 0
			    /* EDX = out2 */
			    || upeek(pid, PT_R10, (long *) &tcp->u_arg[2]) < 0
			    /* ESI = out3 */
			    || upeek(pid, PT_R14, (long *) &tcp->u_arg[3]) < 0
			    /* EDI = out4 */
			    || upeek(pid, PT_R15, (long *) &tcp->u_arg[4]) < 0
			    /* EBP = out5 */
			    || upeek(pid, PT_R13, (long *) &tcp->u_arg[5]) < 0)
				return -1;

			for (i = 0; i < 6; ++i)
				/* truncate away IVE sign-extension */
				tcp->u_arg[i] &= 0xffffffff;

			if (tcp->scno >= 0 && tcp->scno < nsyscalls
			    && sysent[tcp->scno].nargs != -1)
				tcp->u_nargs = sysent[tcp->scno].nargs;
			else
				tcp->u_nargs = 5;
		}
	}
#elif defined (MIPS)
	{
	  	long sp;
	  	int i, nargs;

		if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
			nargs = tcp->u_nargs = sysent[tcp->scno].nargs;
		else 
     	        	nargs = tcp->u_nargs = MAX_ARGS;
		if(nargs > 4) {
		  	if(upeek(pid, REG_SP, &sp) < 0)
			  	return -1;
			for(i = 0; i < 4; i++) {
			  	if (upeek(pid, REG_A0 + i, &tcp->u_arg[i])<0)
				  	return -1;
			}
			umoven(tcp, sp+16, (nargs-4) * sizeof(tcp->u_arg[0]),
			       (char *)(tcp->u_arg + 4));
		} else {
		  	for(i = 0; i < nargs; i++) {
			  	if (upeek(pid, REG_A0 + i, &tcp->u_arg[i]) < 0)
				  	return -1;
			}
		}
	}
#elif defined (POWERPC)
	{
		int i;
		if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
			tcp->u_nargs = sysent[tcp->scno].nargs;
		else 
     	        	tcp->u_nargs = MAX_ARGS;
		for (i = 0; i < tcp->u_nargs; i++) {
			if (upeek(pid, (i==0) ? (4*PT_ORIG_R3) : ((i+PT_R3)*4), &tcp->u_arg[i]) < 0)
				return -1;
		}
	}
#elif defined (SPARC)
	{
		int i;

		if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
			tcp->u_nargs = sysent[tcp->scno].nargs;
		else 
     	        	tcp->u_nargs = MAX_ARGS;
		for (i = 0; i < tcp->u_nargs; i++)
			tcp->u_arg[i] = *((&regs.r_o0) + i);
	}
#elif defined (HPPA)
	{
		int i;

		if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
			tcp->u_nargs = sysent[tcp->scno].nargs;
		else 
     	        	tcp->u_nargs = MAX_ARGS;
		for (i = 0; i < tcp->u_nargs; i++) {
			if (upeek(pid, PT_GR26-4*i, &tcp->u_arg[i]) < 0)
				return -1;
		}
	}
#elif defined(SH)
       {
               int i; 
               static int syscall_regs[] = {
                   REG_REG0+4, REG_REG0+5, REG_REG0+6, REG_REG0+7,
                   REG_REG0, REG_REG0+1, REG_REG0+2
                   };

               tcp->u_nargs = sysent[tcp->scno].nargs;
               for (i = 0; i < tcp->u_nargs; i++) {
                       if (upeek(pid, 4*syscall_regs[i], &tcp->u_arg[i]) < 0)
                               return -1;
               }
        }
#elif defined(X86_64)
	{
		int i;
		static int argreg[SUPPORTED_PERSONALITIES][MAX_ARGS] = {
			{RDI,RSI,RDX,R10,R8,R9},	/* x86-64 ABI */
			{RBX,RCX,RDX,RDX,RSI,RDI,RBP}	/* i386 ABI */
		};
		
		if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
			tcp->u_nargs = sysent[tcp->scno].nargs;
		else 
     	        	tcp->u_nargs = MAX_ARGS;
		for (i = 0; i < tcp->u_nargs; i++) {
			if (upeek(pid, argreg[current_personality][i]*8, &tcp->u_arg[i]) < 0)
				return -1;
		}
	}
#else /* Other architecture (like i386) (32bits specific) */
	{
		int i;
		if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
			tcp->u_nargs = sysent[tcp->scno].nargs;
		else 
     	        	tcp->u_nargs = MAX_ARGS;
		for (i = 0; i < tcp->u_nargs; i++) {
			if (upeek(pid, i*4, &tcp->u_arg[i]) < 0)
				return -1;
		}
	}
#endif 
#endif /* LINUX */
#ifdef SUNOS4
	{
		int i;
		if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
			tcp->u_nargs = sysent[tcp->scno].nargs;
		else 
     	        	tcp->u_nargs = MAX_ARGS;
		for (i = 0; i < tcp->u_nargs; i++) {
			struct user *u;

			if (upeek(pid, uoff(u_arg[0]) +
			    (i*sizeof(u->u_arg[0])), &tcp->u_arg[i]) < 0)
				return -1;
		}
	}
#endif /* SUNOS4 */
#ifdef SVR4
#ifdef MIPS
	/*
	 * SGI is broken: even though it has pr_sysarg, it doesn't
	 * set them on system call entry.  Get a clue.
	 */
	if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
		tcp->u_nargs = sysent[tcp->scno].nargs;
	else
		tcp->u_nargs = tcp->status.pr_nsysarg;
	if (tcp->u_nargs > 4) {
		memcpy(tcp->u_arg, &tcp->status.pr_reg[CTX_A0],
			4*sizeof(tcp->u_arg[0]));
		umoven(tcp, tcp->status.pr_reg[CTX_SP] + 16,
			(tcp->u_nargs - 4)*sizeof(tcp->u_arg[0]), (char *) (tcp->u_arg + 4));
	}
	else {
		memcpy(tcp->u_arg, &tcp->status.pr_reg[CTX_A0],
			tcp->u_nargs*sizeof(tcp->u_arg[0]));
	}
#elif UNIXWARE >= 2
	/*
	 * Like SGI, UnixWare doesn't set pr_sysarg until system call exit
	 */
	if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
		tcp->u_nargs = sysent[tcp->scno].nargs;
	else
		tcp->u_nargs = tcp->status.pr_lwp.pr_nsysarg;
	umoven(tcp, tcp->status.PR_REG[UESP] + 4,
		tcp->u_nargs*sizeof(tcp->u_arg[0]), (char *) tcp->u_arg);
#elif defined (HAVE_PR_SYSCALL)
	if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
		tcp->u_nargs = sysent[tcp->scno].nargs;
	else
		tcp->u_nargs = tcp->status.pr_nsysarg;
	{
		int i;
		for (i = 0; i < tcp->u_nargs; i++)
			tcp->u_arg[i] = tcp->status.pr_sysarg[i];
	}
#elif defined (I386)
	if (tcp->scno >= 0 && tcp->scno < nsyscalls && sysent[tcp->scno].nargs != -1)
		tcp->u_nargs = sysent[tcp->scno].nargs;
	else
		tcp->u_nargs = 5;
	umoven(tcp, tcp->status.PR_REG[UESP] + 4,
		tcp->u_nargs*sizeof(tcp->u_arg[0]), (char *) tcp->u_arg);
#else
	I DONT KNOW WHAT TO DO
#endif /* !HAVE_PR_SYSCALL */
#endif /* SVR4 */
#ifdef FREEBSD
	if (tcp->scno >= 0 && tcp->scno < nsyscalls &&
	    sysent[tcp->scno].nargs > tcp->status.val)
		tcp->u_nargs = sysent[tcp->scno].nargs;
	else 
	  	tcp->u_nargs = tcp->status.val;
	if (tcp->u_nargs < 0)
		tcp->u_nargs = 0;
	if (tcp->u_nargs > MAX_ARGS)
		tcp->u_nargs = MAX_ARGS;
	switch(regs.r_eax) {
	case SYS___syscall:
		pread(tcp->pfd, &tcp->u_arg, tcp->u_nargs * sizeof(unsigned long),
		      regs.r_esp + sizeof(int) + sizeof(quad_t));
	  break;
        case SYS_syscall:
		pread(tcp->pfd, &tcp->u_arg, tcp->u_nargs * sizeof(unsigned long),
		      regs.r_esp + 2 * sizeof(int));
	  break;
        default:
		pread(tcp->pfd, &tcp->u_arg, tcp->u_nargs * sizeof(unsigned long),
		      regs.r_esp + sizeof(int));
	  break;
	}
#endif /* FREEBSD */
	return 1;
}

int
trace_syscall(tcp)
struct tcb *tcp;
{
	int sys_res;
	struct timeval tv;
	int res;

	/* Measure the exit time as early as possible to avoid errors. */
	if (dtime && (tcp->flags & TCB_INSYSCALL))
		gettimeofday(&tv, NULL);

	res = get_scno(tcp);
	if (res != 1)
		return res;

	res = syscall_fixup(tcp);
	if (res != 1)
		return res;

	if (tcp->flags & TCB_INSYSCALL) {
		long u_error;
		res = get_error(tcp);
		if (res != 1)
			return res;
		u_error = tcp->u_error;


		internal_syscall(tcp);
		if (tcp->scno >= 0 && tcp->scno < nsyscalls &&
		    !(qual_flags[tcp->scno] & QUAL_TRACE)) {
			tcp->flags &= ~TCB_INSYSCALL;
			return 0;
		}

		if (tcp->flags & TCB_REPRINT) {
			printleader(tcp);
			tprintf("<... ");
			if (tcp->scno >= nsyscalls || tcp->scno < 0)
				tprintf("syscall_%lu", tcp->scno);
			else
				tprintf("%s", sysent[tcp->scno].sys_name);
			tprintf(" resumed> ");
		}

		if (cflag && tcp->scno < nsyscalls && tcp->scno >= 0) {
			call_count[tcp->scno]++;
			if (tcp->u_error)
				error_count[tcp->scno]++;
			tv_sub(&tv, &tv, &tcp->etime);
#ifdef LINUX
			if (tv_cmp(&tv, &tcp->dtime) > 0) {
				static struct timeval one_tick =
					{ 0, 1000000 / HZ };

				if (tv_nz(&tcp->dtime))
					tv = tcp->dtime;
				else if (tv_cmp(&tv, &one_tick) > 0) {
					if (tv_cmp(&shortest, &one_tick) < 0)
						tv = shortest;
					else
						tv = one_tick;
				}
			}
#endif /* LINUX */
			if (tv_cmp(&tv, &shortest) < 0)
				shortest = tv;
			tv_add(&tv_count[tcp->scno],
				&tv_count[tcp->scno], &tv);
			tcp->flags &= ~TCB_INSYSCALL;
			return 0;
		}

		if (tcp->scno >= nsyscalls || tcp->scno < 0
		    || (qual_flags[tcp->scno] & QUAL_RAW))
			sys_res = printargs(tcp);
		else {
			if (not_failing_only && tcp->u_error)
				return;	/* ignore failed syscalls */
			sys_res = (*sysent[tcp->scno].sys_func)(tcp);
		}	
		u_error = tcp->u_error;
		tprintf(") ");
		tabto(acolumn);
		if (tcp->scno >= nsyscalls || tcp->scno < 0 ||
		    qual_flags[tcp->scno] & QUAL_RAW) {
			if (u_error)
				tprintf("= -1 (errno %ld)", u_error);
			else
				tprintf("= %#lx", tcp->u_rval);
		}
		else if (!(sys_res & RVAL_NONE) && u_error) {
			switch (u_error) {
#ifdef LINUX
			case ERESTARTSYS:
				tprintf("= ? ERESTARTSYS (To be restarted)");
				break;
			case ERESTARTNOINTR:
				tprintf("= ? ERESTARTNOINTR (To be restarted)");
				break;
			case ERESTARTNOHAND:
				tprintf("= ? ERESTARTNOHAND (To be restarted)");
				break;
#endif /* LINUX */
			default:
				tprintf("= -1 ");
				if (u_error < 0)
					tprintf("E??? (errno %ld)", u_error);
				else if (u_error < nerrnos && u_error < sys_nerr)
					tprintf("%s (%s)", errnoent[u_error],
						sys_errlist[u_error]);
				else if (u_error < nerrnos)
					tprintf("%s (errno %ld)",
						errnoent[u_error], u_error);
				else if (u_error < sys_nerr)
					tprintf("ERRNO_%ld (%s)", u_error,
						sys_errlist[u_error]);
				else
					tprintf("E??? (errno %ld)", u_error);
				break;
			}
		}
		else {
			if (sys_res & RVAL_NONE)
				tprintf("= ?");
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
#ifdef HAVE_LONG_LONG
				case RVAL_LHEX:
					tprintf("= %#llx", tcp->u_lrval);
					break;
				case RVAL_LOCTAL:
					tprintf("= %#llo", tcp->u_lrval);
					break;
				case RVAL_LUDECIMAL:
					tprintf("= %llu", tcp->u_lrval);
					break;
				case RVAL_LDECIMAL:
					tprintf("= %lld", tcp->u_lrval);
					break;
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
		if (dtime) {
			tv_sub(&tv, &tv, &tcp->etime);
			tprintf(" <%ld.%06ld>",
				(long) tv.tv_sec, (long) tv.tv_usec);
		}
		printtrailer(tcp);

		dumpio(tcp);
		if (fflush(tcp->outf) == EOF)
			return -1;
		tcp->flags &= ~TCB_INSYSCALL;
		return 0;
	}

	/* Entering system call */
	res = syscall_enter(tcp);
	if (res != 1)
		return res;

	switch (tcp->scno + NR_SYSCALL_BASE) {
#ifdef LINUX
#if !defined (ALPHA) && !defined(SPARC) && !defined(MIPS) && !defined(HPPA) && !defined(X86_64)
	case SYS_socketcall:
		decode_subcall(tcp, SYS_socket_subcall,
			SYS_socket_nsubcalls, deref_style);
		break;
	case SYS_ipc:
		decode_subcall(tcp, SYS_ipc_subcall,
			SYS_ipc_nsubcalls, shift_style);
		break;
#endif /* !ALPHA && !MIPS && !SPARC && !HPPA && !X86_64 */
#ifdef SPARC
	case SYS_socketcall:
		sparc_socket_decode (tcp);
		break;
#endif
#endif /* LINUX */
#ifdef SVR4
#ifdef SYS_pgrpsys_subcall
	case SYS_pgrpsys:
		decode_subcall(tcp, SYS_pgrpsys_subcall,
			SYS_pgrpsys_nsubcalls, shift_style);
		break;
#endif /* SYS_pgrpsys_subcall */
#ifdef SYS_sigcall_subcall
	case SYS_sigcall:
		decode_subcall(tcp, SYS_sigcall_subcall,
			SYS_sigcall_nsubcalls, mask_style);
		break;
#endif /* SYS_sigcall_subcall */
	case SYS_msgsys:
		decode_subcall(tcp, SYS_msgsys_subcall,
			SYS_msgsys_nsubcalls, shift_style);
		break;
	case SYS_shmsys:
		decode_subcall(tcp, SYS_shmsys_subcall,
			SYS_shmsys_nsubcalls, shift_style);
		break;
	case SYS_semsys:
		decode_subcall(tcp, SYS_semsys_subcall,
			SYS_semsys_nsubcalls, shift_style);
		break;
#if 0 /* broken */
	case SYS_utssys:
		decode_subcall(tcp, SYS_utssys_subcall,
			SYS_utssys_nsubcalls, shift_style);
		break;
#endif
	case SYS_sysfs:
		decode_subcall(tcp, SYS_sysfs_subcall,
			SYS_sysfs_nsubcalls, shift_style);
		break;
	case SYS_spcall:
		decode_subcall(tcp, SYS_spcall_subcall,
			SYS_spcall_nsubcalls, shift_style);
		break;
#ifdef SYS_context_subcall
	case SYS_context:
		decode_subcall(tcp, SYS_context_subcall,
			SYS_context_nsubcalls, shift_style);
		break;
#endif /* SYS_context_subcall */
#ifdef SYS_door_subcall
	case SYS_door:
		decode_subcall(tcp, SYS_door_subcall,
			SYS_door_nsubcalls, door_style);
		break;
#endif /* SYS_door_subcall */
#ifdef SYS_kaio_subcall
	case SYS_kaio:
		decode_subcall(tcp, SYS_kaio_subcall,
			SYS_kaio_nsubcalls, shift_style);
		break;
#endif
#endif /* SVR4 */
#ifdef FREEBSD
	case SYS_msgsys:
	case SYS_shmsys:
	case SYS_semsys:
		decode_subcall(tcp, 0, 0, table_style);
		break;
#endif
#ifdef SUNOS4
	case SYS_semsys:
		decode_subcall(tcp, SYS_semsys_subcall,
			SYS_semsys_nsubcalls, shift_style);
		break;
	case SYS_msgsys:
		decode_subcall(tcp, SYS_msgsys_subcall,
			SYS_msgsys_nsubcalls, shift_style);
		break;
	case SYS_shmsys:
		decode_subcall(tcp, SYS_shmsys_subcall,
			SYS_shmsys_nsubcalls, shift_style);
		break;
#endif
	}

	internal_syscall(tcp);
	if (tcp->scno >=0 && tcp->scno < nsyscalls && !(qual_flags[tcp->scno] & QUAL_TRACE)) {
		tcp->flags |= TCB_INSYSCALL;
		return 0;
	}

	if (cflag) {
		gettimeofday(&tcp->etime, NULL);
		tcp->flags |= TCB_INSYSCALL;
		return 0;
	}

	printleader(tcp);
	tcp->flags &= ~TCB_REPRINT;
	tcp_last = tcp;
	if (tcp->scno >= nsyscalls || tcp->scno < 0)
		tprintf("syscall_%lu(", tcp->scno);
	else
		tprintf("%s(", sysent[tcp->scno].sys_name);
	if (tcp->scno >= nsyscalls || tcp->scno < 0 || 
	    ((qual_flags[tcp->scno] & QUAL_RAW) && tcp->scno != SYS_exit))
		sys_res = printargs(tcp);
	else
		sys_res = (*sysent[tcp->scno].sys_func)(tcp);
	if (fflush(tcp->outf) == EOF)
		return -1;
	tcp->flags |= TCB_INSYSCALL;
	/* Measure the entrance time as late as possible to avoid errors. */
	if (dtime)
		gettimeofday(&tcp->etime, NULL);
	return sys_res;
}

int
printargs(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		int i;

		for (i = 0; i < tcp->u_nargs; i++)
			tprintf("%s%#lx", i ? ", " : "", tcp->u_arg[i]);
	}
	return 0;
}

long
getrval2(tcp)
struct tcb *tcp;
{
	long val = -1;

#ifdef LINUX
#ifdef SPARC
	struct regs regs;
	if (ptrace(PTRACE_GETREGS,tcp->pid,(char *)&regs,0) < 0)
		return -1;
	val = regs.r_o1;
#endif /* SPARC */
#endif /* LINUX */

#ifdef SUNOS4
	if (upeek(tcp->pid, uoff(u_rval2), &val) < 0)
		return -1;
#endif /* SUNOS4 */

#ifdef SVR4
#ifdef SPARC
	val = tcp->status.PR_REG[R_O1];
#endif /* SPARC */
#ifdef I386
	val = tcp->status.PR_REG[EDX];
#endif /* I386 */
#ifdef X86_64
	val = tcp->status.PR_REG[RDX];
#endif /* X86_64 */
#ifdef MIPS
	val = tcp->status.PR_REG[CTX_V1];
#endif /* MIPS */
#endif /* SVR4 */
#ifdef FREEBSD
	struct reg regs;
	pread(tcp->pfd_reg, &regs, sizeof(regs), 0);
	val = regs.r_edx;
#endif	
	return val;
}

/*
 * Apparently, indirect system calls have already be converted by ptrace(2),
 * so if you see "indir" this program has gone astray.
 */
int
sys_indir(tcp)
struct tcb *tcp;
{
	int i, scno, nargs;

	if (entering(tcp)) {
		if ((scno = tcp->u_arg[0]) > nsyscalls) {
			fprintf(stderr, "Bogus syscall: %u\n", scno);
			return 0;
		}
		nargs = sysent[scno].nargs;
		tprintf("%s", sysent[scno].sys_name);
		for (i = 0; i < nargs; i++)
			tprintf(", %#lx", tcp->u_arg[i+1]);
	}
	return 0;
}

static int
time_cmp(a, b)
void *a;
void *b;
{
	return -tv_cmp(&tv_count[*((int *) a)], &tv_count[*((int *) b)]);
}

static int
syscall_cmp(a, b)
void *a;
void *b;
{
	return strcmp(sysent[*((int *) a)].sys_name,
		sysent[*((int *) b)].sys_name);
}

static int
count_cmp(a, b)
void *a;
void *b;
{
	int m = call_count[*((int *) a)], n = call_count[*((int *) b)];

	return (m < n) ? 1 : (m > n) ? -1 : 0;
}

static int (*sortfun)();
static struct timeval overhead = { -1, -1 };

void
set_sortby(sortby)
char *sortby;
{
	if (strcmp(sortby, "time") == 0)
		sortfun = time_cmp;
	else if (strcmp(sortby, "calls") == 0)
		sortfun = count_cmp;
	else if (strcmp(sortby, "name") == 0)
		sortfun = syscall_cmp;
	else if (strcmp(sortby, "nothing") == 0)
		sortfun = NULL;
	else {
		fprintf(stderr, "invalid sortby: `%s'\n", sortby);
		exit(1);
	}
}

void set_overhead(n)
int n;
{
	overhead.tv_sec = n / 1000000;
	overhead.tv_usec = n % 1000000;
}

void
call_summary(outf)
FILE *outf;
{
	int i, j;
	int call_cum, error_cum;
	struct timeval tv_cum, dtv;
	double percent;
	char *dashes = "-------------------------";
	char error_str[16];

	call_cum = error_cum = tv_cum.tv_sec = tv_cum.tv_usec = 0;
	if (overhead.tv_sec == -1) {
		tv_mul(&overhead, &shortest, 8);
		tv_div(&overhead, &overhead, 10);
	}
	for (i = 0; i < nsyscalls; i++) {
		sorted_count[i] = i;
		if (call_count[i] == 0)
			continue;
		tv_mul(&dtv, &overhead, call_count[i]);
		tv_sub(&tv_count[i], &tv_count[i], &dtv);
		call_cum += call_count[i];
		error_cum += error_count[i];
		tv_add(&tv_cum, &tv_cum, &tv_count[i]);
	}
	if (sortfun)
		qsort((void *) sorted_count, nsyscalls, sizeof(int), sortfun);
	fprintf(outf, "%6.6s %11.11s %11.11s %9.9s %9.9s %s\n",
		"% time", "seconds", "usecs/call",
		"calls", "errors", "syscall");
	fprintf(outf, "%6.6s %11.11s %11.11s %9.9s %9.9s %-16.16s\n",
		dashes, dashes, dashes, dashes, dashes, dashes);
	for (i = 0; i < nsyscalls; i++) {
		j = sorted_count[i];
		if (call_count[j] == 0)
			continue;
		tv_div(&dtv, &tv_count[j], call_count[j]);
		if (error_count[j])
			sprintf(error_str, "%d", error_count[j]);
		else
			error_str[0] = '\0';
		percent = 100.0*tv_float(&tv_count[j])/tv_float(&tv_cum);
		fprintf(outf, "%6.2f %4ld.%06ld %11ld %9d %9.9s %s\n",
			percent, (long) tv_count[j].tv_sec,
			(long) tv_count[j].tv_usec,
			(long) 1000000 * dtv.tv_sec + dtv.tv_usec,
			call_count[j], error_str, sysent[j].sys_name);
	}
	fprintf(outf, "%6.6s %11.11s %11.11s %9.9s %9.9s %-16.16s\n",
		dashes, dashes, dashes, dashes, dashes, dashes);
	if (error_cum)
		sprintf(error_str, "%d", error_cum);
	else
		error_str[0] = '\0';
	fprintf(outf, "%6.6s %4ld.%06ld %11.11s %9d %9.9s %s\n",
		"100.00", (long) tv_cum.tv_sec, (long) tv_cum.tv_usec, "",
		call_cum, error_str, "total");
}
