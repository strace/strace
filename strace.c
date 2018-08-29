/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2018 The strace developers.
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
#include <stdarg.h>
#include <limits.h>
#include <fcntl.h>
#include "ptrace.h"
#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#ifdef HAVE_PATHS_H
# include <paths.h>
#endif
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <locale.h>
#include <sys/utsname.h>
#ifdef HAVE_PRCTL
# include <sys/prctl.h>
#endif
#include <asm/unistd.h>

#include "largefile_wrappers.h"
#include "mmap_cache.h"
#include "number_set.h"
#include "scno.h"
#include "printsiginfo.h"
#include "trace_event.h"
#include "xstring.h"
#include "delay.h"

/* In some libc, these aren't declared. Do it ourself: */
extern char **environ;
extern int optind;
extern char *optarg;

#ifdef ENABLE_STACKTRACE
/* if this is true do the stack trace for every system call */
bool stack_trace_enabled;
#endif

#define my_tkill(tid, sig) syscall(__NR_tkill, (tid), (sig))

/* Glue for systems without a MMU that cannot provide fork() */
#if !defined(HAVE_FORK)
# undef NOMMU_SYSTEM
# define NOMMU_SYSTEM 1
#endif
#if NOMMU_SYSTEM
# define fork() vfork()
#endif

const unsigned int syscall_trap_sig = SIGTRAP | 0x80;

cflag_t cflag = CFLAG_NONE;
unsigned int followfork;
unsigned int ptrace_setoptions = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXEC
				 | PTRACE_O_TRACEEXIT;
unsigned int xflag;
bool debug_flag;
bool Tflag;
bool iflag;
bool count_wallclock;
unsigned int qflag;
static unsigned int tflag;
static bool rflag;
static bool print_pid_pfx;

/* -I n */
enum {
	INTR_NOT_SET        = 0,
	INTR_ANYWHERE       = 1, /* don't block/ignore any signals */
	INTR_WHILE_WAIT     = 2, /* block fatal signals while decoding syscall. default */
	INTR_NEVER          = 3, /* block fatal signals. default if '-o FILE PROG' */
	INTR_BLOCK_TSTP_TOO = 4, /* block fatal signals and SIGTSTP (^Z) */
	NUM_INTR_OPTS
};
static int opt_intr;
/* We play with signal mask only if this mode is active: */
#define interactive (opt_intr == INTR_WHILE_WAIT)

/*
 * daemonized_tracer supports -D option.
 * With this option, strace forks twice.
 * Unlike normal case, with -D *grandparent* process exec's,
 * becoming a traced process. Child exits (this prevents traced process
 * from having children it doesn't expect to have), and grandchild
 * attaches to grandparent similarly to strace -p PID.
 * This allows for more transparent interaction in cases
 * when process and its parent are communicating via signals,
 * wait() etc. Without -D, strace process gets lodged in between,
 * disrupting parent<->child link.
 */
static bool daemonized_tracer;

static int post_attach_sigstop = TCB_IGNORE_ONE_SIGSTOP;
#define use_seize (post_attach_sigstop == 0)

/* Sometimes we want to print only succeeding syscalls. */
bool not_failing_only;

/* Show path associated with fd arguments */
unsigned int show_fd_path;

static bool detach_on_execve;

static int exit_code;
static int strace_child;
static int strace_tracer_pid;

static const char *username;
static uid_t run_uid;
static gid_t run_gid;

unsigned int max_strlen = DEFAULT_STRLEN;
static int acolumn = DEFAULT_ACOLUMN;
static char *acolumn_spaces;

/* Default output style for xlat entities */
enum xlat_style xlat_verbosity = XLAT_STYLE_ABBREV;

static const char *outfname;
/* If -ff, points to stderr. Else, it's our common output log */
static FILE *shared_log;
static bool open_append;

struct tcb *printing_tcp;
static struct tcb *current_tcp;

struct tcb_wait_data {
	enum trace_event te; /**< Event passed to dispatch_event() */
	int status;          /**< status, returned by wait4() */
	siginfo_t si;        /**< siginfo, returned by PTRACE_GETSIGINFO */
};

static struct tcb **tcbtab;
static unsigned int nprocs;
static size_t tcbtabsize;

#ifndef HAVE_PROGRAM_INVOCATION_NAME
char *program_invocation_name;
#endif

unsigned os_release; /* generated from uname()'s u.release */

static void detach(struct tcb *tcp);
static void cleanup(void);
static void interrupt(int sig);

#ifdef HAVE_SIG_ATOMIC_T
static volatile sig_atomic_t interrupted, restart_failed;
#else
static volatile int interrupted, restart_failed;
#endif

static sigset_t timer_set;
static void timer_sighandler(int);

#ifndef HAVE_STRERROR

#if !HAVE_DECL_SYS_ERRLIST
extern int sys_nerr;
extern char *sys_errlist[];
#endif

const char *
strerror(int err_no)
{
	static char buf[sizeof("Unknown error %d") + sizeof(int)*3];

	if (err_no < 1 || err_no >= sys_nerr) {
		xsprintf(buf, "Unknown error %d", err_no);
		return buf;
	}
	return sys_errlist[err_no];
}

#endif /* HAVE_STERRROR */

static void
print_version(void)
{
	static const char features[] =
#ifdef ENABLE_STACKTRACE
		" stack-trace=" USE_UNWINDER
#endif
#ifdef USE_DEMANGLE
		" stack-demangle"
#endif
#if SUPPORTED_PERSONALITIES > 1
# if defined HAVE_M32_MPERS
		" m32-mpers"
# else
		" no-m32-mpers"
# endif
#endif /* SUPPORTED_PERSONALITIES > 1 */
#if SUPPORTED_PERSONALITIES > 2
# if defined HAVE_MX32_MPERS
		" mx32-mpers"
# else
		" no-mx32-mpers"
# endif
#endif /* SUPPORTED_PERSONALITIES > 2 */
		"";

	printf("%s -- version %s\n"
	       "Copyright (c) 1991-%s The strace developers <%s>.\n"
	       "This is free software; see the source for copying conditions.  There is NO\n"
	       "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	       PACKAGE_NAME, PACKAGE_VERSION, COPYRIGHT_YEAR, PACKAGE_URL);
	printf("\nOptional features enabled:%s\n",
	       features[0] ? features : " (none)");
}

static void
usage(void)
{
	printf("\
usage: strace [-CdffhiqrtttTvVwxxy] [-I n] [-e expr]...\n\
              [-a column] [-o file] [-s strsize] [-P path]...\n\
              -p pid... / [-D] [-E var=val]... [-u username] PROG [ARGS]\n\
   or: strace -c[dfw] [-I n] [-e expr]... [-O overhead] [-S sortby]\n\
              -p pid... / [-D] [-E var=val]... [-u username] PROG [ARGS]\n\
\n\
Output format:\n\
  -a column      alignment COLUMN for printing syscall results (default %d)\n\
  -i             print instruction pointer at time of syscall\n\
"
#ifdef ENABLE_STACKTRACE
"\
  -k             obtain stack trace between each syscall\n\
"
#endif
"\
  -o file        send trace output to FILE instead of stderr\n\
  -q             suppress messages about attaching, detaching, etc.\n\
  -r             print relative timestamp\n\
  -s strsize     limit length of print strings to STRSIZE chars (default %d)\n\
  -t             print absolute timestamp\n\
  -tt            print absolute timestamp with usecs\n\
  -T             print time spent in each syscall\n\
  -x             print non-ascii strings in hex\n\
  -xx            print all strings in hex\n\
  -y             print paths associated with file descriptor arguments\n\
  -yy            print protocol specific information associated with socket file descriptors\n\
\n\
Statistics:\n\
  -c             count time, calls, and errors for each syscall and report summary\n\
  -C             like -c but also print regular output\n\
  -O overhead    set overhead for tracing syscalls to OVERHEAD usecs\n\
  -S sortby      sort syscall counts by: time, calls, name, nothing (default %s)\n\
  -w             summarise syscall latency (default is system time)\n\
\n\
Filtering:\n\
  -e expr        a qualifying expression: option=[!]all or option=[!]val1[,val2]...\n\
     options:    trace, abbrev, verbose, raw, signal, read, write, fault, inject, kvm"
"\n\
  -P path        trace accesses to path\n\
\n\
Tracing:\n\
  -b execve      detach on execve syscall\n\
  -D             run tracer process as a detached grandchild, not as parent\n\
  -f             follow forks\n\
  -ff            follow forks with output into separate files\n\
  -I interruptible\n\
     1:          no signals are blocked\n\
     2:          fatal signals are blocked while decoding syscall (default)\n\
     3:          fatal signals are always blocked (default if '-o FILE PROG')\n\
     4:          fatal signals and SIGTSTP (^Z) are always blocked\n\
                 (useful to make 'strace -o FILE PROG' not stop on ^Z)\n\
\n\
Startup:\n\
  -E var         remove var from the environment for command\n\
  -E var=val     put var=val in the environment for command\n\
  -p pid         trace process with process id PID, may be repeated\n\
  -u username    run command as username handling setuid and/or setgid\n\
\n\
Miscellaneous:\n\
  -d             enable debug output to stderr\n\
  -v             verbose mode: print unabbreviated argv, stat, termios, etc. args\n\
  -h             print help message\n\
  -V             print version\n\
"
/* ancient, no one should use it
-F -- attempt to follow vforks (deprecated, use -f)\n\
 */
/* this is broken, so don't document it
-z -- print only succeeding syscalls\n\
 */
, DEFAULT_ACOLUMN, DEFAULT_STRLEN, DEFAULT_SORTBY);
	exit(0);
}

void ATTRIBUTE_NORETURN
die(void)
{
	if (strace_tracer_pid == getpid()) {
		cflag = 0;
		cleanup();
		exit(1);
	}

	_exit(1);
}

static void
error_opt_arg(int opt, const char *arg)
{
	error_msg_and_help("invalid -%c argument: '%s'", opt, arg);
}

static const char *ptrace_attach_cmd;

static int
ptrace_attach_or_seize(int pid)
{
	int r;
	if (!use_seize)
		return ptrace_attach_cmd = "PTRACE_ATTACH",
		       ptrace(PTRACE_ATTACH, pid, 0L, 0L);
	r = ptrace(PTRACE_SEIZE, pid, 0L, (unsigned long) ptrace_setoptions);
	if (r)
		return ptrace_attach_cmd = "PTRACE_SEIZE", r;
	r = ptrace(PTRACE_INTERRUPT, pid, 0L, 0L);
	return ptrace_attach_cmd = "PTRACE_INTERRUPT", r;
}

/*
 * Used when we want to unblock stopped traced process.
 * Should be only used with PTRACE_CONT, PTRACE_DETACH and PTRACE_SYSCALL.
 * Returns 0 on success or if error was ESRCH
 * (presumably process was killed while we talk to it).
 * Otherwise prints error message and returns -1.
 */
static int
ptrace_restart(const unsigned int op, struct tcb *const tcp, unsigned int sig)
{
	int err;
	const char *msg;

	errno = 0;
	ptrace(op, tcp->pid, 0L, (unsigned long) sig);
	err = errno;
	if (!err)
		return 0;

	switch (op) {
		case PTRACE_CONT:
			msg = "CONT";
			break;
		case PTRACE_DETACH:
			msg = "DETACH";
			break;
		case PTRACE_LISTEN:
			msg = "LISTEN";
			break;
		default:
			msg = "SYSCALL";
	}

	/*
	 * Why curcol != 0? Otherwise sometimes we get this:
	 *
	 * 10252 kill(10253, SIGKILL)              = 0
	 *  <ptrace(SYSCALL,10252):No such process>10253 ...next decode...
	 *
	 * 10252 died after we retrieved syscall exit data,
	 * but before we tried to restart it. Log looks ugly.
	 */
	if (current_tcp && current_tcp->curcol != 0) {
		tprintf(" <ptrace(%s):%s>\n", msg, strerror(err));
		line_ended();
	}
	if (err == ESRCH)
		return 0;
	errno = err;
	perror_msg("ptrace(PTRACE_%s,pid:%d,sig:%u)", msg, tcp->pid, sig);
	return -1;
}

static void
set_cloexec_flag(int fd)
{
	int flags, newflags;

	flags = fcntl(fd, F_GETFD);
	if (flags < 0) {
		/* Can happen only if fd is bad.
		 * Should never happen: if it does, we have a bug
		 * in the caller. Therefore we just abort
		 * instead of propagating the error.
		 */
		perror_msg_and_die("fcntl(%d, F_GETFD)", fd);
	}

	newflags = flags | FD_CLOEXEC;
	if (flags == newflags)
		return;

	if (fcntl(fd, F_SETFD, newflags)) /* never fails */
		perror_msg_and_die("fcntl(%d, F_SETFD, %#x)", fd, newflags);
}

static void
kill_save_errno(pid_t pid, int sig)
{
	int saved_errno = errno;

	(void) kill(pid, sig);
	errno = saved_errno;
}

/*
 * When strace is setuid executable, we have to swap uids
 * before and after filesystem and process management operations.
 */
static void
swap_uid(void)
{
	int euid = geteuid(), uid = getuid();

	if (euid != uid && setreuid(euid, uid) < 0) {
		perror_msg_and_die("setreuid");
	}
}

static FILE *
strace_fopen(const char *path)
{
	FILE *fp;

	swap_uid();
	fp = fopen_stream(path, open_append ? "a" : "w");
	if (!fp)
		perror_msg_and_die("Can't fopen '%s'", path);
	swap_uid();
	set_cloexec_flag(fileno(fp));
	return fp;
}

static int popen_pid;

#ifndef _PATH_BSHELL
# define _PATH_BSHELL "/bin/sh"
#endif

/*
 * We cannot use standard popen(3) here because we have to distinguish
 * popen child process from other processes we trace, and standard popen(3)
 * does not export its child's pid.
 */
static FILE *
strace_popen(const char *command)
{
	FILE *fp;
	int pid;
	int fds[2];

	swap_uid();
	if (pipe(fds) < 0)
		perror_msg_and_die("pipe");

	set_cloexec_flag(fds[1]); /* never fails */

	pid = vfork();
	if (pid < 0)
		perror_msg_and_die("vfork");

	if (pid == 0) {
		/* child */
		close(fds[1]);
		if (fds[0] != 0) {
			if (dup2(fds[0], 0))
				perror_msg_and_die("dup2");
			close(fds[0]);
		}
		execl(_PATH_BSHELL, "sh", "-c", command, NULL);
		perror_msg_and_die("Can't execute '%s'", _PATH_BSHELL);
	}

	/* parent */
	popen_pid = pid;
	close(fds[0]);
	swap_uid();
	fp = fdopen(fds[1], "w");
	if (!fp)
		perror_msg_and_die("fdopen");
	return fp;
}

static void
outf_perror(const struct tcb * const tcp)
{
	if (tcp->outf == stderr)
		return;

	/* This is ugly, but we don't store separate file names */
	if (followfork >= 2)
		perror_msg("%s.%u", outfname, tcp->pid);
	else
		perror_msg("%s", outfname);
}

ATTRIBUTE_FORMAT((printf, 1, 0))
static void
tvprintf(const char *const fmt, va_list args)
{
	if (current_tcp) {
		int n = vfprintf(current_tcp->outf, fmt, args);
		if (n < 0) {
			/* very unlikely due to vfprintf buffering */
			outf_perror(current_tcp);
		} else
			current_tcp->curcol += n;
	}
}

void
tprintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	tvprintf(fmt, args);
	va_end(args);
}

#ifndef HAVE_FPUTS_UNLOCKED
# define fputs_unlocked fputs
#endif

void
tprints(const char *str)
{
	if (current_tcp) {
		int n = fputs_unlocked(str, current_tcp->outf);
		if (n >= 0) {
			current_tcp->curcol += strlen(str);
			return;
		}
		/* very unlikely due to fputs_unlocked buffering */
		outf_perror(current_tcp);
	}
}

void
tprints_comment(const char *const str)
{
	if (str && *str)
		tprintf(" /* %s */", str);
}

void
tprintf_comment(const char *fmt, ...)
{
	if (!fmt || !*fmt)
		return;

	va_list args;
	va_start(args, fmt);
	tprints(" /* ");
	tvprintf(fmt, args);
	tprints(" */");
	va_end(args);
}

static void
flush_tcp_output(const struct tcb *const tcp)
{
	if (fflush(tcp->outf))
		outf_perror(tcp);
}

void
line_ended(void)
{
	if (current_tcp) {
		current_tcp->curcol = 0;
		flush_tcp_output(current_tcp);
	}
	if (printing_tcp) {
		printing_tcp->curcol = 0;
		printing_tcp = NULL;
	}
}

void
set_current_tcp(const struct tcb *tcp)
{
	current_tcp = (struct tcb *) tcp;

	/* Sync current_personality and stuff */
	if (current_tcp)
		set_personality(current_tcp->currpers);
}

void
printleader(struct tcb *tcp)
{
	/* If -ff, "previous tcb we printed" is always the same as current,
	 * because we have per-tcb output files.
	 */
	if (followfork >= 2)
		printing_tcp = tcp;

	if (printing_tcp) {
		set_current_tcp(printing_tcp);
		if (printing_tcp->curcol != 0 && (followfork < 2 || printing_tcp == tcp)) {
			/*
			 * case 1: we have a shared log (i.e. not -ff), and last line
			 * wasn't finished (same or different tcb, doesn't matter).
			 * case 2: split log, we are the same tcb, but our last line
			 * didn't finish ("SIGKILL nuked us after syscall entry" etc).
			 */
			tprints(" <unfinished ...>\n");
			printing_tcp->curcol = 0;
		}
	}

	printing_tcp = tcp;
	set_current_tcp(tcp);
	current_tcp->curcol = 0;

	if (print_pid_pfx)
		tprintf("%-5d ", tcp->pid);
	else if (nprocs > 1 && !outfname)
		tprintf("[pid %5u] ", tcp->pid);

	if (tflag) {
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);

		if (tflag > 2) {
			tprintf("%lld.%06ld ",
				(long long) ts.tv_sec, (long) ts.tv_nsec / 1000);
		} else {
			time_t local = ts.tv_sec;
			char str[MAX(sizeof("HH:MM:SS"), sizeof(ts.tv_sec) * 3)];
			struct tm *tm = localtime(&local);

			if (tm)
				strftime(str, sizeof(str), "%T", tm);
			else
				xsprintf(str, "%lld", (long long) local);
			if (tflag > 1)
				tprintf("%s.%06ld ",
					str, (long) ts.tv_nsec / 1000);
			else
				tprintf("%s ", str);
		}
	}

	if (rflag) {
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);

		static struct timespec ots;
		if (ots.tv_sec == 0)
			ots = ts;

		struct timespec dts;
		ts_sub(&dts, &ts, &ots);
		ots = ts;

		tprintf("%s%6ld.%06ld%s ",
			tflag ? "(+" : "",
			(long) dts.tv_sec, (long) dts.tv_nsec / 1000,
			tflag ? ")" : "");
	}

	if (iflag)
		print_pc(tcp);
}

void
tabto(void)
{
	if (current_tcp->curcol < acolumn)
		tprints(acolumn_spaces + current_tcp->curcol);
}

/* Should be only called directly *after successful attach* to a tracee.
 * Otherwise, "strace -oFILE -ff -p<nonexistant_pid>"
 * may create bogus empty FILE.<nonexistant_pid>, and then die.
 */
static void
after_successful_attach(struct tcb *tcp, const unsigned int flags)
{
	tcp->flags |= TCB_ATTACHED | TCB_STARTUP | flags;
	tcp->outf = shared_log; /* if not -ff mode, the same file is for all */
	if (followfork >= 2) {
		char name[PATH_MAX];
		xsprintf(name, "%s.%u", outfname, tcp->pid);
		tcp->outf = strace_fopen(name);
	}

#ifdef ENABLE_STACKTRACE
	if (stack_trace_enabled)
		unwind_tcb_init(tcp);
#endif
}

static void
expand_tcbtab(void)
{
	/* Allocate some (more) TCBs (and expand the table).
	   We don't want to relocate the TCBs because our
	   callers have pointers and it would be a pain.
	   So tcbtab is a table of pointers.  Since we never
	   free the TCBs, we allocate a single chunk of many.  */
	size_t old_tcbtabsize;
	struct tcb *newtcbs;
	struct tcb **tcb_ptr;

	old_tcbtabsize = tcbtabsize;

	tcbtab = xgrowarray(tcbtab, &tcbtabsize, sizeof(tcbtab[0]));
	newtcbs = xcalloc(tcbtabsize - old_tcbtabsize, sizeof(newtcbs[0]));

	for (tcb_ptr = tcbtab + old_tcbtabsize;
	    tcb_ptr < tcbtab + tcbtabsize; tcb_ptr++, newtcbs++)
		*tcb_ptr = newtcbs;
}

static struct tcb *
alloctcb(int pid)
{
	unsigned int i;
	struct tcb *tcp;

	if (nprocs == tcbtabsize)
		expand_tcbtab();

	for (i = 0; i < tcbtabsize; i++) {
		tcp = tcbtab[i];
		if (!tcp->pid) {
			memset(tcp, 0, sizeof(*tcp));
			tcp->pid = pid;
#if SUPPORTED_PERSONALITIES > 1
			tcp->currpers = current_personality;
#endif
			nprocs++;
			debug_msg("new tcb for pid %d, active tcbs:%d",
				  tcp->pid, nprocs);
			return tcp;
		}
	}
	error_msg_and_die("bug in alloctcb");
}

void *
get_tcb_priv_data(const struct tcb *tcp)
{
	return tcp->_priv_data;
}

int
set_tcb_priv_data(struct tcb *tcp, void *const priv_data,
		  void (*const free_priv_data)(void *))
{
	if (tcp->_priv_data)
		return -1;

	tcp->_free_priv_data = free_priv_data;
	tcp->_priv_data = priv_data;

	return 0;
}

void
free_tcb_priv_data(struct tcb *tcp)
{
	if (tcp->_priv_data) {
		if (tcp->_free_priv_data) {
			tcp->_free_priv_data(tcp->_priv_data);
			tcp->_free_priv_data = NULL;
		}
		tcp->_priv_data = NULL;
	}
}

static void
droptcb(struct tcb *tcp)
{
	if (tcp->pid == 0)
		return;

	int p;
	for (p = 0; p < SUPPORTED_PERSONALITIES; ++p)
		free(tcp->inject_vec[p]);

	free_tcb_priv_data(tcp);

#ifdef ENABLE_STACKTRACE
	if (stack_trace_enabled)
		unwind_tcb_fin(tcp);
#endif

#ifdef HAVE_LINUX_KVM_H
	kvm_vcpu_info_free(tcp);
#endif

	if (tcp->mmap_cache)
		tcp->mmap_cache->free_fn(tcp, __func__);

	nprocs--;
	debug_msg("dropped tcb for pid %d, %d remain", tcp->pid, nprocs);

	if (tcp->outf) {
		if (followfork >= 2) {
			if (tcp->curcol != 0)
				fprintf(tcp->outf, " <detached ...>\n");
			fclose(tcp->outf);
		} else {
			if (printing_tcp == tcp && tcp->curcol != 0)
				fprintf(tcp->outf, " <detached ...>\n");
			flush_tcp_output(tcp);
		}
	}

	if (current_tcp == tcp)
		set_current_tcp(NULL);
	if (printing_tcp == tcp)
		printing_tcp = NULL;

	memset(tcp, 0, sizeof(*tcp));
}

/* Detach traced process.
 * Never call DETACH twice on the same process as both unattached and
 * attached-unstopped processes give the same ESRCH.  For unattached process we
 * would SIGSTOP it and wait for its SIGSTOP notification forever.
 */
static void
detach(struct tcb *tcp)
{
	int error;
	int status;

	/*
	 * Linux wrongly insists the child be stopped
	 * before detaching.  Arghh.  We go through hoops
	 * to make a clean break of things.
	 */

	if (!(tcp->flags & TCB_ATTACHED))
		goto drop;

	/* We attached but possibly didn't see the expected SIGSTOP.
	 * We must catch exactly one as otherwise the detached process
	 * would be left stopped (process state T).
	 */
	if (tcp->flags & TCB_IGNORE_ONE_SIGSTOP)
		goto wait_loop;

	error = ptrace(PTRACE_DETACH, tcp->pid, 0, 0);
	if (!error) {
		/* On a clear day, you can see forever. */
		goto drop;
	}
	if (errno != ESRCH) {
		/* Shouldn't happen. */
		perror_func_msg("ptrace(PTRACE_DETACH,%u)", tcp->pid);
		goto drop;
	}
	/* ESRCH: process is either not stopped or doesn't exist. */
	if (my_tkill(tcp->pid, 0) < 0) {
		if (errno != ESRCH)
			/* Shouldn't happen. */
			perror_func_msg("tkill(%u,0)", tcp->pid);
		/* else: process doesn't exist. */
		goto drop;
	}
	/* Process is not stopped, need to stop it. */
	if (use_seize) {
		/*
		 * With SEIZE, tracee can be in group-stop already.
		 * In this state sending it another SIGSTOP does nothing.
		 * Need to use INTERRUPT.
		 * Testcase: trying to ^C a "strace -p <stopped_process>".
		 */
		error = ptrace(PTRACE_INTERRUPT, tcp->pid, 0, 0);
		if (!error)
			goto wait_loop;
		if (errno != ESRCH)
			perror_func_msg("ptrace(PTRACE_INTERRUPT,%u)", tcp->pid);
	} else {
		error = my_tkill(tcp->pid, SIGSTOP);
		if (!error)
			goto wait_loop;
		if (errno != ESRCH)
			perror_func_msg("tkill(%u,SIGSTOP)", tcp->pid);
	}
	/* Either process doesn't exist, or some weird error. */
	goto drop;

 wait_loop:
	/* We end up here in three cases:
	 * 1. We sent PTRACE_INTERRUPT (use_seize case)
	 * 2. We sent SIGSTOP (!use_seize)
	 * 3. Attach SIGSTOP was already pending (TCB_IGNORE_ONE_SIGSTOP set)
	 */
	for (;;) {
		unsigned int sig;
		if (waitpid(tcp->pid, &status, __WALL) < 0) {
			if (errno == EINTR)
				continue;
			/*
			 * if (errno == ECHILD) break;
			 * ^^^  WRONG! We expect this PID to exist,
			 * and want to emit a message otherwise:
			 */
			perror_func_msg("waitpid(%u)", tcp->pid);
			break;
		}
		if (!WIFSTOPPED(status)) {
			/*
			 * Tracee exited or was killed by signal.
			 * We shouldn't normally reach this place:
			 * we don't want to consume exit status.
			 * Consider "strace -p PID" being ^C-ed:
			 * we want merely to detach from PID.
			 *
			 * However, we _can_ end up here if tracee
			 * was SIGKILLed.
			 */
			break;
		}
		sig = WSTOPSIG(status);
		debug_msg("detach wait: event:%d sig:%d",
			  (unsigned) status >> 16, sig);
		if (use_seize) {
			unsigned event = (unsigned)status >> 16;
			if (event == PTRACE_EVENT_STOP /*&& sig == SIGTRAP*/) {
				/*
				 * sig == SIGTRAP: PTRACE_INTERRUPT stop.
				 * sig == other: process was already stopped
				 * with this stopping sig (see tests/detach-stopped).
				 * Looks like re-injecting this sig is not necessary
				 * in DETACH for the tracee to remain stopped.
				 */
				sig = 0;
			}
			/*
			 * PTRACE_INTERRUPT is not guaranteed to produce
			 * the above event if other ptrace-stop is pending.
			 * See tests/detach-sleeping testcase:
			 * strace got SIGINT while tracee is sleeping.
			 * We sent PTRACE_INTERRUPT.
			 * We see syscall exit, not PTRACE_INTERRUPT stop.
			 * We won't get PTRACE_INTERRUPT stop
			 * if we would CONT now. Need to DETACH.
			 */
			if (sig == syscall_trap_sig)
				sig = 0;
			/* else: not sure in which case we can be here.
			 * Signal stop? Inject it while detaching.
			 */
			ptrace_restart(PTRACE_DETACH, tcp, sig);
			break;
		}
		/* Note: this check has to be after use_seize check */
		/* (else, in use_seize case SIGSTOP will be mistreated) */
		if (sig == SIGSTOP) {
			/* Detach, suppressing SIGSTOP */
			ptrace_restart(PTRACE_DETACH, tcp, 0);
			break;
		}
		if (sig == syscall_trap_sig)
			sig = 0;
		/* Can't detach just yet, may need to wait for SIGSTOP */
		error = ptrace_restart(PTRACE_CONT, tcp, sig);
		if (error < 0) {
			/* Should not happen.
			 * Note: ptrace_restart returns 0 on ESRCH, so it's not it.
			 * ptrace_restart already emitted error message.
			 */
			break;
		}
	}

 drop:
	if (!qflag && (tcp->flags & TCB_ATTACHED))
		error_msg("Process %u detached", tcp->pid);

	droptcb(tcp);
}

static void
process_opt_p_list(char *opt)
{
	while (*opt) {
		/*
		 * We accept -p PID,PID; -p "`pidof PROG`"; -p "`pgrep PROG`".
		 * pidof uses space as delim, pgrep uses newline. :(
		 */
		int pid;
		char *delim = opt + strcspn(opt, "\n\t ,");
		char c = *delim;

		*delim = '\0';
		pid = string_to_uint(opt);
		if (pid <= 0) {
			error_msg_and_die("Invalid process id: '%s'", opt);
		}
		if (pid == strace_tracer_pid) {
			error_msg_and_die("I'm sorry, I can't let you do that, Dave.");
		}
		*delim = c;
		alloctcb(pid);
		if (c == '\0')
			break;
		opt = delim + 1;
	}
}

static void
attach_tcb(struct tcb *const tcp)
{
	if (ptrace_attach_or_seize(tcp->pid) < 0) {
		perror_msg("attach: ptrace(%s, %d)",
			   ptrace_attach_cmd, tcp->pid);
		droptcb(tcp);
		return;
	}

	after_successful_attach(tcp, TCB_GRABBED | post_attach_sigstop);
	debug_msg("attach to pid %d (main) succeeded", tcp->pid);

	static const char task_path[] = "/proc/%d/task";
	char procdir[sizeof(task_path) + sizeof(int) * 3];
	DIR *dir;
	unsigned int ntid = 0, nerr = 0;

	if (followfork && tcp->pid != strace_child &&
	    xsprintf(procdir, task_path, tcp->pid) > 0 &&
	    (dir = opendir(procdir)) != NULL) {
		struct_dirent *de;

		while ((de = read_dir(dir)) != NULL) {
			if (de->d_fileno == 0)
				continue;

			int tid = string_to_uint(de->d_name);
			if (tid <= 0 || tid == tcp->pid)
				continue;

			++ntid;
			if (ptrace_attach_or_seize(tid) < 0) {
				++nerr;
				debug_perror_msg("attach: ptrace(%s, %d)",
						 ptrace_attach_cmd, tid);
				continue;
			}

			after_successful_attach(alloctcb(tid),
						TCB_GRABBED | post_attach_sigstop);
			debug_msg("attach to pid %d succeeded", tid);
		}

		closedir(dir);
	}

	if (!qflag) {
		if (ntid > nerr)
			error_msg("Process %u attached"
				  " with %u threads",
				  tcp->pid, ntid - nerr + 1);
		else
			error_msg("Process %u attached",
				  tcp->pid);
	}
}

static void
startup_attach(void)
{
	pid_t parent_pid = strace_tracer_pid;
	unsigned int tcbi;
	struct tcb *tcp;

	if (daemonized_tracer) {
		pid_t pid = fork();
		if (pid < 0)
			perror_func_msg_and_die("fork");

		if (pid) { /* parent */
			/*
			 * Wait for grandchild to attach to straced process
			 * (grandparent). Grandchild SIGKILLs us after it attached.
			 * Grandparent's wait() is unblocked by our death,
			 * it proceeds to exec the straced program.
			 */
			pause();
			_exit(0); /* paranoia */
		}
		/* grandchild */
		/* We will be the tracer process. Remember our new pid: */
		strace_tracer_pid = getpid();
	}

	for (tcbi = 0; tcbi < tcbtabsize; tcbi++) {
		tcp = tcbtab[tcbi];

		if (!tcp->pid)
			continue;

		/* Is this a process we should attach to, but not yet attached? */
		if (tcp->flags & TCB_ATTACHED)
			continue; /* no, we already attached it */

		if (tcp->pid == parent_pid || tcp->pid == strace_tracer_pid) {
			errno = EPERM;
			perror_msg("attach: pid %d", tcp->pid);
			droptcb(tcp);
			continue;
		}

		attach_tcb(tcp);

		if (interrupted)
			return;
	} /* for each tcbtab[] */

	if (daemonized_tracer) {
		/*
		 * Make parent go away.
		 * Also makes grandparent's wait() unblock.
		 */
		kill(parent_pid, SIGKILL);
		strace_child = 0;
	}
}

/* Stack-o-phobic exec helper, in the hope to work around
 * NOMMU + "daemonized tracer" difficulty.
 */
struct exec_params {
	int fd_to_close;
	uid_t run_euid;
	gid_t run_egid;
	char **argv;
	char *pathname;
	struct sigaction child_sa;
};
static struct exec_params params_for_tracee;

static void ATTRIBUTE_NOINLINE ATTRIBUTE_NORETURN
exec_or_die(void)
{
	struct exec_params *params = &params_for_tracee;

	if (params->fd_to_close >= 0)
		close(params->fd_to_close);
	if (!daemonized_tracer && !use_seize) {
		if (ptrace(PTRACE_TRACEME, 0L, 0L, 0L) < 0) {
			perror_msg_and_die("ptrace(PTRACE_TRACEME, ...)");
		}
	}

	if (username != NULL) {
		/*
		 * It is important to set groups before we
		 * lose privileges on setuid.
		 */
		if (initgroups(username, run_gid) < 0) {
			perror_msg_and_die("initgroups");
		}
		if (setregid(run_gid, params->run_egid) < 0) {
			perror_msg_and_die("setregid");
		}
		if (setreuid(run_uid, params->run_euid) < 0) {
			perror_msg_and_die("setreuid");
		}
	} else if (geteuid() != 0)
		if (setreuid(run_uid, run_uid) < 0) {
			perror_msg_and_die("setreuid");
		}

	if (!daemonized_tracer) {
		/*
		 * Induce a ptrace stop. Tracer (our parent)
		 * will resume us with PTRACE_SYSCALL and display
		 * the immediately following execve syscall.
		 * Can't do this on NOMMU systems, we are after
		 * vfork: parent is blocked, stopping would deadlock.
		 */
		if (!NOMMU_SYSTEM)
			kill(getpid(), SIGSTOP);
	} else {
		alarm(3);
		/* we depend on SIGCHLD set to SIG_DFL by init code */
		/* if it happens to be SIG_IGN'ed, wait won't block */
		wait(NULL);
		alarm(0);
	}

	if (params_for_tracee.child_sa.sa_handler != SIG_DFL)
		sigaction(SIGCHLD, &params_for_tracee.child_sa, NULL);

	execv(params->pathname, params->argv);
	perror_msg_and_die("exec");
}

/*
 * Open a dummy descriptor for use as a placeholder.
 * The descriptor is O_RDONLY with FD_CLOEXEC flag set.
 * A read attempt from such descriptor ends with EOF,
 * a write attempt is rejected with EBADF.
 */
static int
open_dummy_desc(void)
{
	int fds[2];

	if (pipe(fds))
		perror_func_msg_and_die("pipe");
	close(fds[1]);
	set_cloexec_flag(fds[0]);
	return fds[0];
}

/* placeholder fds status for stdin and stdout */
static bool fd_is_placeholder[2];

/*
 * Ensure that all standard file descriptors are open by opening placeholder
 * file descriptors for those standard file descriptors that are not open.
 *
 * The information which descriptors have been made open is saved
 * in fd_is_placeholder for later use.
 */
static void
ensure_standard_fds_opened(void)
{
	int fd;

	while ((fd = open_dummy_desc()) <= 2) {
		if (fd == 2)
			break;
		fd_is_placeholder[fd] = true;
	}

	if (fd > 2)
		close(fd);
}

/*
 * Redirect stdin and stdout unless they have been opened earlier
 * by ensure_standard_fds_opened as placeholders.
 */
static void
redirect_standard_fds(void)
{
	int i;

	/*
	 * It might be a good idea to redirect stderr as well,
	 * but we sometimes need to print error messages.
	 */
	for (i = 0; i <= 1; ++i) {
		if (!fd_is_placeholder[i]) {
			close(i);
			open_dummy_desc();
		}
	}
}

static void
startup_child(char **argv)
{
	struct_stat statbuf;
	const char *filename;
	size_t filename_len;
	char pathname[PATH_MAX];
	int pid;
	struct tcb *tcp;

	filename = argv[0];
	filename_len = strlen(filename);

	if (filename_len > sizeof(pathname) - 1) {
		errno = ENAMETOOLONG;
		perror_msg_and_die("exec");
	}
	if (strchr(filename, '/')) {
		strcpy(pathname, filename);
	}
#ifdef USE_DEBUGGING_EXEC
	/*
	 * Debuggers customarily check the current directory
	 * first regardless of the path but doing that gives
	 * security geeks a panic attack.
	 */
	else if (stat_file(filename, &statbuf) == 0)
		strcpy(pathname, filename);
#endif /* USE_DEBUGGING_EXEC */
	else {
		const char *path;
		size_t m, n, len;

		for (path = getenv("PATH"); path && *path; path += m) {
			const char *colon = strchr(path, ':');
			if (colon) {
				n = colon - path;
				m = n + 1;
			} else
				m = n = strlen(path);
			if (n == 0) {
				if (!getcwd(pathname, PATH_MAX))
					continue;
				len = strlen(pathname);
			} else if (n > sizeof(pathname) - 1)
				continue;
			else {
				strncpy(pathname, path, n);
				len = n;
			}
			if (len && pathname[len - 1] != '/')
				pathname[len++] = '/';
			if (filename_len + len > sizeof(pathname) - 1)
				continue;
			strcpy(pathname + len, filename);
			if (stat_file(pathname, &statbuf) == 0 &&
			    /* Accept only regular files
			       with some execute bits set.
			       XXX not perfect, might still fail */
			    S_ISREG(statbuf.st_mode) &&
			    (statbuf.st_mode & 0111))
				break;
		}
		if (!path || !*path)
			pathname[0] = '\0';
	}
	if (stat_file(pathname, &statbuf) < 0) {
		perror_msg_and_die("Can't stat '%s'", filename);
	}

	params_for_tracee.fd_to_close = (shared_log != stderr) ? fileno(shared_log) : -1;
	params_for_tracee.run_euid = (statbuf.st_mode & S_ISUID) ? statbuf.st_uid : run_uid;
	params_for_tracee.run_egid = (statbuf.st_mode & S_ISGID) ? statbuf.st_gid : run_gid;
	params_for_tracee.argv = argv;
	/*
	 * On NOMMU, can be safely freed only after execve in tracee.
	 * It's hard to know when that happens, so we just leak it.
	 */
	params_for_tracee.pathname = NOMMU_SYSTEM ? xstrdup(pathname) : pathname;

#if defined HAVE_PRCTL && defined PR_SET_PTRACER && defined PR_SET_PTRACER_ANY
	if (daemonized_tracer)
		prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
#endif

	pid = fork();
	if (pid < 0)
		perror_func_msg_and_die("fork");

	if ((pid != 0 && daemonized_tracer)
	 || (pid == 0 && !daemonized_tracer)
	) {
		/* We are to become the tracee. Two cases:
		 * -D: we are parent
		 * not -D: we are child
		 */
		exec_or_die();
	}

	/* We are the tracer */

	if (!daemonized_tracer) {
		strace_child = pid;
		if (!use_seize) {
			/* child did PTRACE_TRACEME, nothing to do in parent */
		} else {
			if (!NOMMU_SYSTEM) {
				/* Wait until child stopped itself */
				int status;
				while (waitpid(pid, &status, WSTOPPED) < 0) {
					if (errno == EINTR)
						continue;
					perror_msg_and_die("waitpid");
				}
				if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP) {
					kill_save_errno(pid, SIGKILL);
					perror_msg_and_die("Unexpected wait status %#x",
							   status);
				}
			}
			/* Else: NOMMU case, we have no way to sync.
			 * Just attach to it as soon as possible.
			 * This means that we may miss a few first syscalls...
			 */

			if (ptrace_attach_or_seize(pid)) {
				kill_save_errno(pid, SIGKILL);
				perror_msg_and_die("attach: ptrace(%s, %d)",
						   ptrace_attach_cmd, pid);
			}
			if (!NOMMU_SYSTEM)
				kill(pid, SIGCONT);
		}
		tcp = alloctcb(pid);
		after_successful_attach(tcp, TCB_SKIP_DETACH_ON_FIRST_EXEC
					     | (NOMMU_SYSTEM ? 0
						: (TCB_HIDE_LOG
						   | post_attach_sigstop)));
	} else {
		/* With -D, we are *child* here, the tracee is our parent. */
		strace_child = strace_tracer_pid;
		strace_tracer_pid = getpid();
		tcp = alloctcb(strace_child);
		tcp->flags |= TCB_SKIP_DETACH_ON_FIRST_EXEC | TCB_HIDE_LOG;
		/*
		 * Attaching will be done later, by startup_attach.
		 * Note: we don't do after_successful_attach() here either!
		 */

		/* NOMMU BUG! -D mode is active, we (child) return,
		 * and we will scribble over parent's stack!
		 * When parent later unpauses, it segfaults.
		 *
		 * We work around it
		 * (1) by declaring exec_or_die() NORETURN,
		 * hopefully compiler will just jump to it
		 * instead of call (won't push anything to stack),
		 * (2) by trying very hard in exec_or_die()
		 * to not use any stack,
		 * (3) having a really big (PATH_MAX) stack object
		 * in this function, which creates a "buffer" between
		 * child's and parent's stack pointers.
		 * This may save us if (1) and (2) failed
		 * and compiler decided to use stack in exec_or_die() anyway
		 * (happens on i386 because of stack parameter passing).
		 *
		 * A cleaner solution is to use makecontext + setcontext
		 * to create a genuine separate stack and execute on it.
		 */
	}
	/*
	 * A case where straced process is part of a pipe:
	 * { sleep 1; yes | head -n99999; } | strace -o/dev/null sh -c 'exec <&-; sleep 9'
	 * If strace won't close its fd#0, closing it in tracee is not enough:
	 * the pipe is still open, it has a reader. Thus, "head" will not get its
	 * SIGPIPE at once, on the first write.
	 *
	 * Preventing it by redirecting strace's stdin/out.
	 * (Don't leave fds 0 and 1 closed, this is bad practice: future opens
	 * will reuse them, unexpectedly making a newly opened object "stdin").
	 */
	redirect_standard_fds();
}

static void
test_ptrace_seize(void)
{
	int pid;

	/* Need fork for test. NOMMU has no forks */
	if (NOMMU_SYSTEM) {
		post_attach_sigstop = 0; /* this sets use_seize to 1 */
		return;
	}

	pid = fork();
	if (pid < 0)
		perror_func_msg_and_die("fork");

	if (pid == 0) {
		pause();
		_exit(0);
	}

	/* PTRACE_SEIZE, unlike ATTACH, doesn't force tracee to trap.  After
	 * attaching tracee continues to run unless a trap condition occurs.
	 * PTRACE_SEIZE doesn't affect signal or group stop state.
	 */
	if (ptrace(PTRACE_SEIZE, pid, 0, 0) == 0) {
		post_attach_sigstop = 0; /* this sets use_seize to 1 */
	} else {
		debug_msg("PTRACE_SEIZE doesn't work");
	}

	kill(pid, SIGKILL);

	while (1) {
		int status, tracee_pid;

		errno = 0;
		tracee_pid = waitpid(pid, &status, 0);
		if (tracee_pid <= 0) {
			if (errno == EINTR)
				continue;
			perror_func_msg_and_die("unexpected wait result %d",
						tracee_pid);
		}
		if (WIFSIGNALED(status))
			return;

		error_func_msg_and_die("unexpected wait status %#x", status);
	}
}

static unsigned
get_os_release(void)
{
	unsigned rel;
	const char *p;
	struct utsname u;
	if (uname(&u) < 0)
		perror_msg_and_die("uname");
	/* u.release has this form: "3.2.9[-some-garbage]" */
	rel = 0;
	p = u.release;
	for (;;) {
		if (!(*p >= '0' && *p <= '9'))
			error_msg_and_die("Bad OS release string: '%s'", u.release);
		/* Note: this open-codes KERNEL_VERSION(): */
		rel = (rel << 8) | atoi(p);
		if (rel >= KERNEL_VERSION(1, 0, 0))
			break;
		while (*p >= '0' && *p <= '9')
			p++;
		if (*p != '.') {
			if (rel >= KERNEL_VERSION(0, 1, 0)) {
				/* "X.Y-something" means "X.Y.0" */
				rel <<= 8;
				break;
			}
			error_msg_and_die("Bad OS release string: '%s'", u.release);
		}
		p++;
	}
	return rel;
}

static void
set_sighandler(int signo, void (*sighandler)(int), struct sigaction *oldact)
{
	const struct sigaction sa = { .sa_handler = sighandler };
	sigaction(signo, &sa, oldact);
}

/*
 * Initialization part of main() was eating much stack (~0.5k),
 * which was unused after init.
 * We can reuse it if we move init code into a separate function.
 *
 * Don't want main() to inline us and defeat the reason
 * we have a separate function.
 */
static void ATTRIBUTE_NOINLINE
init(int argc, char *argv[])
{
	int c, i;
	int optF = 0;

	if (!program_invocation_name || !*program_invocation_name) {
		static char name[] = "strace";
		program_invocation_name =
			(argc > 0 && argv[0] && *argv[0]) ? argv[0] : name;
	}

	strace_tracer_pid = getpid();

	os_release = get_os_release();

	shared_log = stderr;
	set_sortby(DEFAULT_SORTBY);
	set_personality(DEFAULT_PERSONALITY);
	qualify("trace=all");
	qualify("abbrev=all");
	qualify("verbose=all");
#if DEFAULT_QUAL_FLAGS != (QUAL_TRACE | QUAL_ABBREV | QUAL_VERBOSE)
# error Bug in DEFAULT_QUAL_FLAGS
#endif
	qualify("signal=all");
	while ((c = getopt(argc, argv, "+"
#ifdef ENABLE_STACKTRACE
	    "k"
#endif
	    "a:Ab:cCdDe:E:fFhiI:o:O:p:P:qrs:S:tTu:vVwxX:yz")) != EOF) {
		switch (c) {
		case 'a':
			acolumn = string_to_uint(optarg);
			if (acolumn < 0)
				error_opt_arg(c, optarg);
			break;
		case 'A':
			open_append = true;
			break;
		case 'b':
			if (strcmp(optarg, "execve") != 0)
				error_msg_and_die("Syscall '%s' for -b isn't supported",
					optarg);
			detach_on_execve = 1;
			break;
		case 'c':
			if (cflag == CFLAG_BOTH) {
				error_msg_and_help("-c and -C are mutually exclusive");
			}
			cflag = CFLAG_ONLY_STATS;
			break;
		case 'C':
			if (cflag == CFLAG_ONLY_STATS) {
				error_msg_and_help("-c and -C are mutually exclusive");
			}
			cflag = CFLAG_BOTH;
			break;
		case 'd':
			debug_flag = 1;
			break;
		case 'D':
			daemonized_tracer = 1;
			break;
		case 'e':
			qualify(optarg);
			break;
		case 'E':
			if (putenv(optarg) < 0)
				perror_msg_and_die("putenv");
			break;
		case 'f':
			followfork++;
			break;
		case 'F':
			optF = 1;
			break;
		case 'h':
			usage();
			break;
		case 'i':
			iflag = 1;
			break;
		case 'I':
			opt_intr = string_to_uint_upto(optarg, NUM_INTR_OPTS - 1);
			if (opt_intr <= 0)
				error_opt_arg(c, optarg);
			break;
#ifdef ENABLE_STACKTRACE
		case 'k':
			stack_trace_enabled = true;
			break;
#endif
		case 'o':
			outfname = optarg;
			break;
		case 'O':
			i = string_to_uint(optarg);
			if (i < 0)
				error_opt_arg(c, optarg);
			set_overhead(i);
			break;
		case 'p':
			process_opt_p_list(optarg);
			break;
		case 'P':
			pathtrace_select(optarg);
			break;
		case 'q':
			qflag++;
			break;
		case 'r':
			rflag = 1;
			break;
		case 's':
			i = string_to_uint(optarg);
			if (i < 0 || (unsigned int) i > -1U / 4)
				error_opt_arg(c, optarg);
			max_strlen = i;
			break;
		case 'S':
			set_sortby(optarg);
			break;
		case 't':
			tflag++;
			break;
		case 'T':
			Tflag = 1;
			break;
		case 'u':
			username = optarg;
			break;
		case 'v':
			qualify("abbrev=none");
			break;
		case 'V':
			print_version();
			exit(0);
			break;
		case 'w':
			count_wallclock = 1;
			break;
		case 'x':
			xflag++;
			break;
		case 'X':
			if (!strcmp(optarg, "raw"))
				xlat_verbosity = XLAT_STYLE_RAW;
			else if (!strcmp(optarg, "abbrev"))
				xlat_verbosity = XLAT_STYLE_ABBREV;
			else if (!strcmp(optarg, "verbose"))
				xlat_verbosity = XLAT_STYLE_VERBOSE;
			else
				error_opt_arg(c, optarg);
			break;
		case 'y':
			show_fd_path++;
			break;
		case 'z':
			not_failing_only = 1;
			break;
		default:
			error_msg_and_help(NULL);
			break;
		}
	}

	argv += optind;
	argc -= optind;

	if (argc < 0 || (!nprocs && !argc)) {
		error_msg_and_help("must have PROG [ARGS] or -p PID");
	}

	if (!argc && daemonized_tracer) {
		error_msg_and_help("PROG [ARGS] must be specified with -D");
	}

	if (optF) {
		if (followfork) {
			error_msg("deprecated option -F ignored");
		} else {
			error_msg("option -F is deprecated, "
				  "please use -f instead");
			followfork = optF;
		}
	}

	if (followfork >= 2 && cflag) {
		error_msg_and_help("(-c or -C) and -ff are mutually exclusive");
	}

	if (count_wallclock && !cflag) {
		error_msg_and_help("-w must be given with (-c or -C)");
	}

	if (cflag == CFLAG_ONLY_STATS) {
		if (iflag)
			error_msg("-%c has no effect with -c", 'i');
#ifdef ENABLE_STACKTRACE
		if (stack_trace_enabled)
			error_msg("-%c has no effect with -c", 'k');
#endif
		if (rflag)
			error_msg("-%c has no effect with -c", 'r');
		if (tflag)
			error_msg("-%c has no effect with -c", 't');
		if (Tflag)
			error_msg("-%c has no effect with -c", 'T');
		if (show_fd_path)
			error_msg("-%c has no effect with -c", 'y');
	}

	acolumn_spaces = xmalloc(acolumn + 1);
	memset(acolumn_spaces, ' ', acolumn);
	acolumn_spaces[acolumn] = '\0';

	set_sighandler(SIGCHLD, SIG_DFL, &params_for_tracee.child_sa);

#ifdef ENABLE_STACKTRACE
	if (stack_trace_enabled)
		unwind_init();
#endif

	/* See if they want to run as another user. */
	if (username != NULL) {
		struct passwd *pent;

		if (getuid() != 0 || geteuid() != 0) {
			error_msg_and_die("You must be root to use the -u option");
		}
		pent = getpwnam(username);
		if (pent == NULL) {
			error_msg_and_die("Cannot find user '%s'", username);
		}
		run_uid = pent->pw_uid;
		run_gid = pent->pw_gid;
	} else {
		run_uid = getuid();
		run_gid = getgid();
	}

	if (followfork)
		ptrace_setoptions |= PTRACE_O_TRACECLONE |
				     PTRACE_O_TRACEFORK |
				     PTRACE_O_TRACEVFORK;
	debug_msg("ptrace_setoptions = %#x", ptrace_setoptions);
	test_ptrace_seize();

	/*
	 * Is something weird with our stdin and/or stdout -
	 * for example, may they be not open? In this case,
	 * ensure that none of the future opens uses them.
	 *
	 * This was seen in the wild when /proc/sys/kernel/core_pattern
	 * was set to "|/bin/strace -o/tmp/LOG PROG":
	 * kernel runs coredump helper with fd#0 open but fd#1 closed (!),
	 * therefore LOG gets opened to fd#1, and fd#1 is closed by
	 * "don't hold up stdin/out open" code soon after.
	 */
	ensure_standard_fds_opened();

	/* Check if they want to redirect the output. */
	if (outfname) {
		/* See if they want to pipe the output. */
		if (outfname[0] == '|' || outfname[0] == '!') {
			/*
			 * We can't do the <outfname>.PID funny business
			 * when using popen, so prohibit it.
			 */
			if (followfork >= 2)
				error_msg_and_help("piping the output and -ff "
						   "are mutually exclusive");
			shared_log = strace_popen(outfname + 1);
		} else if (followfork < 2) {
			shared_log = strace_fopen(outfname);
		} else if (strlen(outfname) >= PATH_MAX - sizeof(int) * 3) {
			errno = ENAMETOOLONG;
			perror_msg_and_die("%s", outfname);
		}
	} else {
		/* -ff without -o FILE is the same as single -f */
		if (followfork >= 2)
			followfork = 1;
	}

	if (!outfname || outfname[0] == '|' || outfname[0] == '!') {
		setvbuf(shared_log, NULL, _IOLBF, 0);
	}

	/*
	 * argv[0]	-pPID	-oFILE	Default interactive setting
	 * yes		*	0	INTR_WHILE_WAIT
	 * no		1	0	INTR_WHILE_WAIT
	 * yes		*	1	INTR_NEVER
	 * no		1	1	INTR_WHILE_WAIT
	 */

	if (outfname && argc) {
		if (!opt_intr)
			opt_intr = INTR_NEVER;
		if (!qflag)
			qflag = 1;
	}
	if (!opt_intr)
		opt_intr = INTR_WHILE_WAIT;

	/*
	 * startup_child() must be called before the signal handlers get
	 * installed below as they are inherited into the spawned process.
	 * Also we do not need to be protected by them as during interruption
	 * in the startup_child() mode we kill the spawned process anyway.
	 */
	if (argc) {
		startup_child(argv);
	}

	set_sighandler(SIGTTOU, SIG_IGN, NULL);
	set_sighandler(SIGTTIN, SIG_IGN, NULL);
	if (opt_intr != INTR_ANYWHERE) {
		if (opt_intr == INTR_BLOCK_TSTP_TOO)
			set_sighandler(SIGTSTP, SIG_IGN, NULL);
		/*
		 * In interactive mode (if no -o OUTFILE, or -p PID is used),
		 * fatal signals are handled asynchronously and acted
		 * when waiting for process state changes.
		 * In non-interactive mode these signals are ignored.
		 */
		set_sighandler(SIGHUP, interactive ? interrupt : SIG_IGN, NULL);
		set_sighandler(SIGINT, interactive ? interrupt : SIG_IGN, NULL);
		set_sighandler(SIGQUIT, interactive ? interrupt : SIG_IGN, NULL);
		set_sighandler(SIGPIPE, interactive ? interrupt : SIG_IGN, NULL);
		set_sighandler(SIGTERM, interactive ? interrupt : SIG_IGN, NULL);
	}

	sigemptyset(&timer_set);
	sigaddset(&timer_set, SIGALRM);
	sigprocmask(SIG_BLOCK, &timer_set, NULL);
	set_sighandler(SIGALRM, timer_sighandler, NULL);

	if (nprocs != 0 || daemonized_tracer)
		startup_attach();

	/* Do we want pids printed in our -o OUTFILE?
	 * -ff: no (every pid has its own file); or
	 * -f: yes (there can be more pids in the future); or
	 * -p PID1,PID2: yes (there are already more than one pid)
	 */
	print_pid_pfx = (outfname && followfork < 2 && (followfork == 1 || nprocs > 1));
}

static struct tcb *
pid2tcb(const int pid)
{
	if (pid <= 0)
		return NULL;

#define PID2TCB_CACHE_SIZE 1024U
#define PID2TCB_CACHE_MASK (PID2TCB_CACHE_SIZE - 1)

	static struct tcb *pid2tcb_cache[PID2TCB_CACHE_SIZE];
	struct tcb **const ptcp = &pid2tcb_cache[pid & PID2TCB_CACHE_MASK];
	struct tcb *tcp = *ptcp;

	if (tcp && tcp->pid == pid)
		return tcp;

	for (unsigned int i = 0; i < tcbtabsize; ++i) {
		tcp = tcbtab[i];
		if (tcp->pid == pid)
			return *ptcp = tcp;
	}

	return NULL;
}

static void
cleanup(void)
{
	unsigned int i;
	struct tcb *tcp;
	int fatal_sig;

	/* 'interrupted' is a volatile object, fetch it only once */
	fatal_sig = interrupted;
	if (!fatal_sig)
		fatal_sig = SIGTERM;

	for (i = 0; i < tcbtabsize; i++) {
		tcp = tcbtab[i];
		if (!tcp->pid)
			continue;
		debug_func_msg("looking at pid %u", tcp->pid);
		if (tcp->pid == strace_child) {
			kill(tcp->pid, SIGCONT);
			kill(tcp->pid, fatal_sig);
		}
		detach(tcp);
	}
	if (cflag)
		call_summary(shared_log);
}

static void
interrupt(int sig)
{
	interrupted = sig;
}

static void
print_debug_info(const int pid, int status)
{
	const unsigned int event = (unsigned int) status >> 16;
	char buf[sizeof("WIFEXITED,exitcode=%u") + sizeof(int)*3 /*paranoia:*/ + 16];
	char evbuf[sizeof(",EVENT_VFORK_DONE (%u)") + sizeof(int)*3 /*paranoia:*/ + 16];

	strcpy(buf, "???");
	if (WIFSIGNALED(status))
#ifdef WCOREDUMP
		xsprintf(buf, "WIFSIGNALED,%ssig=%s",
				WCOREDUMP(status) ? "core," : "",
				signame(WTERMSIG(status)));
#else
		xsprintf(buf, "WIFSIGNALED,sig=%s",
				signame(WTERMSIG(status)));
#endif
	if (WIFEXITED(status))
		xsprintf(buf, "WIFEXITED,exitcode=%u", WEXITSTATUS(status));
	if (WIFSTOPPED(status))
		xsprintf(buf, "WIFSTOPPED,sig=%s", signame(WSTOPSIG(status)));
	evbuf[0] = '\0';
	if (event != 0) {
		static const char *const event_names[] = {
			[PTRACE_EVENT_CLONE] = "CLONE",
			[PTRACE_EVENT_FORK]  = "FORK",
			[PTRACE_EVENT_VFORK] = "VFORK",
			[PTRACE_EVENT_VFORK_DONE] = "VFORK_DONE",
			[PTRACE_EVENT_EXEC]  = "EXEC",
			[PTRACE_EVENT_EXIT]  = "EXIT",
			/* [PTRACE_EVENT_STOP (=128)] would make biggish array */
		};
		const char *e = "??";
		if (event < ARRAY_SIZE(event_names))
			e = event_names[event];
		else if (event == PTRACE_EVENT_STOP)
			e = "STOP";
		xsprintf(evbuf, ",EVENT_%s (%u)", e, event);
	}
	error_msg("[wait(0x%06x) = %u] %s%s", status, pid, buf, evbuf);
}

static struct tcb *
maybe_allocate_tcb(const int pid, int status)
{
	if (!WIFSTOPPED(status)) {
		if (detach_on_execve && pid == strace_child) {
			/* example: strace -bexecve sh -c 'exec true' */
			strace_child = 0;
			return NULL;
		}
		/*
		 * This can happen if we inherited an unknown child.
		 * Example: (sleep 1 & exec strace true)
		 */
		error_msg("Exit of unknown pid %u ignored", pid);
		return NULL;
	}
	if (followfork) {
		/* We assume it's a fork/vfork/clone child */
		struct tcb *tcp = alloctcb(pid);
		after_successful_attach(tcp, post_attach_sigstop);
		if (!qflag)
			error_msg("Process %d attached", pid);
		return tcp;
	} else {
		/*
		 * This can happen if a clone call misused CLONE_PTRACE itself.
		 *
		 * There used to be a dance around possible re-injection of
		 * WSTOPSIG(status), but it was later removed as the only
		 * observable stop here is the initial ptrace-stop.
		 */
		ptrace(PTRACE_DETACH, pid, NULL, 0L);
		error_msg("Detached unknown pid %d", pid);
		return NULL;
	}
}

static struct tcb *
maybe_switch_tcbs(struct tcb *tcp, const int pid)
{
	FILE *fp;
	struct tcb *execve_thread;
	long old_pid = 0;

	if (ptrace(PTRACE_GETEVENTMSG, pid, NULL, &old_pid) < 0)
		return tcp;
	/* Avoid truncation in pid2tcb() param passing */
	if (old_pid <= 0 || old_pid == pid)
		return tcp;
	if ((unsigned long) old_pid > UINT_MAX)
		return tcp;
	execve_thread = pid2tcb(old_pid);
	/* It should be !NULL, but I feel paranoid */
	if (!execve_thread)
		return tcp;

	if (execve_thread->curcol != 0) {
		/*
		 * One case we are here is -ff:
		 * try "strace -oLOG -ff test/threaded_execve"
		 */
		fprintf(execve_thread->outf, " <pid changed to %d ...>\n", pid);
		/*execve_thread->curcol = 0; - no need, see code below */
	}
	/* Swap output FILEs (needed for -ff) */
	fp = execve_thread->outf;
	execve_thread->outf = tcp->outf;
	tcp->outf = fp;
	/* And their column positions */
	execve_thread->curcol = tcp->curcol;
	tcp->curcol = 0;
	/* Drop leader, but close execve'd thread outfile (if -ff) */
	droptcb(tcp);
	/* Switch to the thread, reusing leader's outfile and pid */
	tcp = execve_thread;
	tcp->pid = pid;
	if (cflag != CFLAG_ONLY_STATS) {
		printleader(tcp);
		tprintf("+++ superseded by execve in pid %lu +++\n", old_pid);
		line_ended();
		tcp->flags |= TCB_REPRINT;
	}

	return tcp;
}

static void
print_signalled(struct tcb *tcp, const int pid, int status)
{
	if (pid == strace_child) {
		exit_code = 0x100 | WTERMSIG(status);
		strace_child = 0;
	}

	if (cflag != CFLAG_ONLY_STATS
	    && is_number_in_set(WTERMSIG(status), signal_set)) {
		printleader(tcp);
#ifdef WCOREDUMP
		tprintf("+++ killed by %s %s+++\n",
			signame(WTERMSIG(status)),
			WCOREDUMP(status) ? "(core dumped) " : "");
#else
		tprintf("+++ killed by %s +++\n",
			signame(WTERMSIG(status)));
#endif
		line_ended();
	}
}

static void
print_exited(struct tcb *tcp, const int pid, int status)
{
	if (pid == strace_child) {
		exit_code = WEXITSTATUS(status);
		strace_child = 0;
	}

	if (cflag != CFLAG_ONLY_STATS &&
	    qflag < 2) {
		printleader(tcp);
		tprintf("+++ exited with %d +++\n", WEXITSTATUS(status));
		line_ended();
	}
}

static void
print_stopped(struct tcb *tcp, const siginfo_t *si, const unsigned int sig)
{
	if (cflag != CFLAG_ONLY_STATS
	    && !hide_log(tcp)
	    && is_number_in_set(sig, signal_set)) {
		printleader(tcp);
		if (si) {
			tprintf("--- %s ", signame(sig));
			printsiginfo(si);
			tprints(" ---\n");
		} else
			tprintf("--- stopped by %s ---\n", signame(sig));
		line_ended();

#ifdef ENABLE_STACKTRACE
		if (stack_trace_enabled)
			unwind_tcb_print(tcp);
#endif
	}
}

static void
startup_tcb(struct tcb *tcp)
{
	debug_msg("pid %d has TCB_STARTUP, initializing it", tcp->pid);

	tcp->flags &= ~TCB_STARTUP;

	if (!use_seize) {
		debug_msg("setting opts 0x%x on pid %d",
			  ptrace_setoptions, tcp->pid);
		if (ptrace(PTRACE_SETOPTIONS, tcp->pid, NULL, ptrace_setoptions) < 0) {
			if (errno != ESRCH) {
				/* Should never happen, really */
				perror_msg_and_die("PTRACE_SETOPTIONS");
			}
		}
	}

	if ((tcp->flags & TCB_GRABBED) && (get_scno(tcp) == 1))
		tcp->s_prev_ent = tcp->s_ent;
}

static void
print_event_exit(struct tcb *tcp)
{
	if (entering(tcp) || filtered(tcp) || hide_log(tcp)
	    || cflag == CFLAG_ONLY_STATS) {
		return;
	}

	if (followfork < 2 && printing_tcp && printing_tcp != tcp
	    && printing_tcp->curcol != 0) {
		set_current_tcp(printing_tcp);
		tprints(" <unfinished ...>\n");
		flush_tcp_output(printing_tcp);
		printing_tcp->curcol = 0;
		set_current_tcp(tcp);
	}

	if ((followfork < 2 && printing_tcp != tcp)
	    || (tcp->flags & TCB_REPRINT)) {
		tcp->flags &= ~TCB_REPRINT;
		printleader(tcp);
		tprintf("<... %s resumed>", tcp->s_ent->sys_name);
	}

	if (!(tcp->sys_func_rval & RVAL_DECODED)) {
		/*
		 * The decoder has probably decided to print something
		 * on exiting syscall which is not going to happen.
		 */
		tprints(" <unfinished ...>");
	}

	printing_tcp = tcp;
	tprints(") ");
	tabto();
	tprints("= ?\n");
	line_ended();
}

static const struct tcb_wait_data *
next_event(void)
{
	static struct tcb_wait_data wait_data;

	int pid;
	int status;
	struct tcb *tcp;
	struct tcb_wait_data *wd = &wait_data;
	struct rusage ru;

	if (interrupted)
		return NULL;

	/*
	 * Used to exit simply when nprocs hits zero, but in this testcase:
	 *  int main(void) { _exit(!!fork()); }
	 * under strace -f, parent sometimes (rarely) manages
	 * to exit before we see the first stop of the child,
	 * and we are losing track of it:
	 *  19923 clone(...) = 19924
	 *  19923 exit_group(1)     = ?
	 *  19923 +++ exited with 1 +++
	 * Exiting only when wait() returns ECHILD works better.
	 */
	if (popen_pid != 0) {
		/* However, if -o|logger is in use, we can't do that.
		 * Can work around that by double-forking the logger,
		 * but that loses the ability to wait for its completion
		 * on exit. Oh well...
		 */
		if (nprocs == 0)
			return NULL;
	}

	const bool unblock_delay_timer = is_delay_timer_armed();

	/*
	 * The window of opportunity to handle expirations
	 * of the delay timer opens here.
	 *
	 * Unblock the signal handler for the delay timer
	 * iff the delay timer is already created.
	 */
	if (unblock_delay_timer)
		sigprocmask(SIG_UNBLOCK, &timer_set, NULL);

	/*
	 * If the delay timer has expired, then its expiration
	 * has been handled already by the signal handler.
	 *
	 * If the delay timer expires during wait4(),
	 * then the system call will be interrupted and
	 * the expiration will be handled by the signal handler.
	 */
	pid = wait4(-1, &status, __WALL, (cflag ? &ru : NULL));
	const int wait_errno = errno;

	/*
	 * The window of opportunity to handle expirations
	 * of the delay timer closes here.
	 *
	 * Block the signal handler for the delay timer
	 * iff it was unblocked earlier.
	 */
	if (unblock_delay_timer) {
		sigprocmask(SIG_BLOCK, &timer_set, NULL);

		if (restart_failed)
			return NULL;
	}

	if (pid < 0) {
		if (wait_errno == EINTR) {
			wd->te = TE_NEXT;
			return wd;
		}
		if (nprocs == 0 && wait_errno == ECHILD)
			return NULL;
		/*
		 * If nprocs > 0, ECHILD is not expected,
		 * treat it as any other error here:
		 */
		errno = wait_errno;
		perror_msg_and_die("wait4(__WALL)");
	}

	wd->status = status;

	if (pid == popen_pid) {
		if (!WIFSTOPPED(status))
			popen_pid = 0;
		wd->te = TE_NEXT;
		return wd;
	}

	if (debug_flag)
		print_debug_info(pid, status);

	/* Look up 'pid' in our table. */
	tcp = pid2tcb(pid);

	if (!tcp) {
		tcp = maybe_allocate_tcb(pid, status);
		if (!tcp) {
			wd->te = TE_NEXT;
			return wd;
		}
	}

	clear_regs(tcp);

	/* Set current output file */
	set_current_tcp(tcp);

	if (cflag) {
		struct timespec stime = {
			.tv_sec = ru.ru_stime.tv_sec,
			.tv_nsec = ru.ru_stime.tv_usec * 1000
		};
		ts_sub(&tcp->dtime, &stime, &tcp->stime);
		tcp->stime = stime;
	}

	if (WIFSIGNALED(status)) {
		wd->te = TE_SIGNALLED;
		return wd;
	}

	if (WIFEXITED(status)) {
		wd->te = TE_EXITED;
		return wd;
	}

	/*
	 * As WCONTINUED flag has not been specified to wait4,
	 * it cannot be WIFCONTINUED(status), so the only case
	 * that remains is WIFSTOPPED(status).
	 */

	/* Is this the very first time we see this tracee stopped? */
	if (tcp->flags & TCB_STARTUP)
		startup_tcb(tcp);

	const unsigned int sig = WSTOPSIG(status);
	const unsigned int event = (unsigned int) status >> 16;

	switch (event) {
	case 0:
		/*
		 * Is this post-attach SIGSTOP?
		 * Interestingly, the process may stop
		 * with STOPSIG equal to some other signal
		 * than SIGSTOP if we happened to attach
		 * just before the process takes a signal.
		 */
		if (sig == SIGSTOP && (tcp->flags & TCB_IGNORE_ONE_SIGSTOP)) {
			debug_func_msg("ignored SIGSTOP on pid %d", tcp->pid);
			tcp->flags &= ~TCB_IGNORE_ONE_SIGSTOP;
			wd->te = TE_RESTART;
		} else if (sig == syscall_trap_sig) {
			wd->te = TE_SYSCALL_STOP;
		} else {
			memset(&wd->si, 0, sizeof(wd->si));
			/*
			 * True if tracee is stopped by signal
			 * (as opposed to "tracee received signal").
			 * TODO: shouldn't we check for errno == EINVAL too?
			 * We can get ESRCH instead, you know...
			 */
			bool stopped = ptrace(PTRACE_GETSIGINFO, pid, 0, &wd->si) < 0;
			wd->te = stopped ? TE_GROUP_STOP : TE_SIGNAL_DELIVERY_STOP;
		}
		break;
	case PTRACE_EVENT_STOP:
		/*
		 * PTRACE_INTERRUPT-stop or group-stop.
		 * PTRACE_INTERRUPT-stop has sig == SIGTRAP here.
		 */
		switch (sig) {
		case SIGSTOP:
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
			wd->te = TE_GROUP_STOP;
			break;
		default:
			wd->te = TE_RESTART;
		}
		break;
	case PTRACE_EVENT_EXEC:
		wd->te = TE_STOP_BEFORE_EXECVE;
		break;
	case PTRACE_EVENT_EXIT:
		wd->te = TE_STOP_BEFORE_EXIT;
		break;
	default:
		wd->te = TE_RESTART;
	}

	return wd;
}

static int
trace_syscall(struct tcb *tcp, unsigned int *sig)
{
	if (entering(tcp)) {
		int res = syscall_entering_decode(tcp);
		switch (res) {
		case 0:
			return 0;
		case 1:
			res = syscall_entering_trace(tcp, sig);
		}
		syscall_entering_finish(tcp, res);
		return res;
	} else {
		struct timespec ts = {};
		int res = syscall_exiting_decode(tcp, &ts);
		if (res != 0) {
			res = syscall_exiting_trace(tcp, &ts, res);
		}
		syscall_exiting_finish(tcp);
		return res;
	}
}

/* Returns true iff the main trace loop has to continue. */
static bool
dispatch_event(const struct tcb_wait_data *wd)
{
	unsigned int restart_op = PTRACE_SYSCALL;
	unsigned int restart_sig = 0;
	enum trace_event te = wd ? wd->te : TE_BREAK;
	/*
	 * Copy wd->status to a non-const variable to workaround glibc bugs
	 * around union wait fixed by glibc commit glibc-2.24~391
	 */
	int status = wd ? wd->status : 0;

	switch (te) {
	case TE_BREAK:
		return false;

	case TE_NEXT:
		return true;

	case TE_RESTART:
		break;

	case TE_SYSCALL_STOP:
		if (trace_syscall(current_tcp, &restart_sig) < 0) {
			/*
			 * ptrace() failed in trace_syscall().
			 * Likely a result of process disappearing mid-flight.
			 * Observed case: exit_group() or SIGKILL terminating
			 * all processes in thread group.
			 * We assume that ptrace error was caused by process death.
			 * We used to detach(current_tcp) here, but since we no
			 * longer implement "detach before death" policy/hack,
			 * we can let this process to report its death to us
			 * normally, via WIFEXITED or WIFSIGNALED wait status.
			 */
			return true;
		}
		break;

	case TE_SIGNAL_DELIVERY_STOP:
		restart_sig = WSTOPSIG(status);
		print_stopped(current_tcp, &wd->si, restart_sig);
		break;

	case TE_SIGNALLED:
		print_signalled(current_tcp, current_tcp->pid, status);
		droptcb(current_tcp);
		return true;

	case TE_GROUP_STOP:
		restart_sig = WSTOPSIG(status);
		print_stopped(current_tcp, NULL, restart_sig);
		if (use_seize) {
			/*
			 * This ends ptrace-stop, but does *not* end group-stop.
			 * This makes stopping signals work properly on straced
			 * process (that is, process really stops. It used to
			 * continue to run).
			 */
			restart_op = PTRACE_LISTEN;
			restart_sig = 0;
		}
		break;

	case TE_EXITED:
		print_exited(current_tcp, current_tcp->pid, status);
		droptcb(current_tcp);
		return true;

	case TE_STOP_BEFORE_EXECVE:
		/*
		 * Check that we are inside syscall now (next event after
		 * PTRACE_EVENT_EXEC should be for syscall exiting).  If it is
		 * not the case, we might have a situation when we attach to a
		 * process and the first thing we see is a PTRACE_EVENT_EXEC
		 * and all the following syscall state tracking is screwed up
		 * otherwise.
		 */
		if (entering(current_tcp)) {
			int ret;

			error_msg("Stray PTRACE_EVENT_EXEC from pid %d"
				  ", trying to recover...",
				  current_tcp->pid);

			current_tcp->flags |= TCB_RECOVERING;
			ret = trace_syscall(current_tcp, &restart_sig);
			current_tcp->flags &= ~TCB_RECOVERING;

			if (ret < 0) {
				/* The reason is described in TE_SYSCALL_STOP */
				return true;
			}
		}

		/*
		 * Under Linux, execve changes pid to thread leader's pid,
		 * and we see this changed pid on EVENT_EXEC and later,
		 * execve sysexit. Leader "disappears" without exit
		 * notification. Let user know that, drop leader's tcb,
		 * and fix up pid in execve thread's tcb.
		 * Effectively, execve thread's tcb replaces leader's tcb.
		 *
		 * BTW, leader is 'stuck undead' (doesn't report WIFEXITED
		 * on exit syscall) in multithreaded programs exactly
		 * in order to handle this case.
		 *
		 * PTRACE_GETEVENTMSG returns old pid starting from Linux 3.0.
		 * On 2.6 and earlier, it can return garbage.
		 */
		if (os_release >= KERNEL_VERSION(3, 0, 0))
			set_current_tcp(maybe_switch_tcbs(current_tcp,
							  current_tcp->pid));

		if (detach_on_execve) {
			if (current_tcp->flags & TCB_SKIP_DETACH_ON_FIRST_EXEC) {
				current_tcp->flags &= ~TCB_SKIP_DETACH_ON_FIRST_EXEC;
			} else {
				detach(current_tcp); /* do "-b execve" thingy */
				return true;
			}
		}
		break;

	case TE_STOP_BEFORE_EXIT:
		print_event_exit(current_tcp);
		break;
	}

	/* We handled quick cases, we are permitted to interrupt now. */
	if (interrupted)
		return false;

	/* If the process is being delayed, do not ptrace_restart just yet */
	if (syscall_delayed(current_tcp))
		return true;

	if (ptrace_restart(restart_op, current_tcp, restart_sig) < 0) {
		/* Note: ptrace_restart emitted error message */
		exit_code = 1;
		return false;
	}
	return true;
}

static bool
restart_delayed_tcb(struct tcb *const tcp)
{
	const struct tcb_wait_data wd = { .te = TE_RESTART };

	debug_func_msg("pid %d", tcp->pid);

	tcp->flags &= ~TCB_DELAYED;

	struct tcb *const prev_tcp = current_tcp;
	current_tcp = tcp;
	bool ret = dispatch_event(&wd);
	current_tcp = prev_tcp;

	return ret;
}

static bool
restart_delayed_tcbs(void)
{
	struct tcb *tcp_next = NULL;
	struct timespec ts_now;

	clock_gettime(CLOCK_MONOTONIC, &ts_now);

	for (size_t i = 0; i < tcbtabsize; i++) {
		struct tcb *tcp = tcbtab[i];

		if (tcp->pid && syscall_delayed(tcp)) {
			if (ts_cmp(&ts_now, &tcp->delay_expiration_time) > 0) {
				if (!restart_delayed_tcb(tcp))
					return false;
			} else {
				/* Check whether this tcb is the next.  */
				if (!tcp_next ||
				    ts_cmp(&tcp_next->delay_expiration_time,
					   &tcp->delay_expiration_time) > 0) {
					tcp_next = tcp;
				}
			}
		}
	}

	if (tcp_next)
		arm_delay_timer(tcp_next);

	return true;
}

/*
 * As this signal handler does a lot of work that is not suitable
 * for signal handlers, extra care must be taken to ensure that
 * it is enabled only in those places where it's safe.
 */
static void
timer_sighandler(int sig)
{
	delay_timer_expired();

	if (restart_failed)
		return;

	int saved_errno = errno;

	if (!restart_delayed_tcbs())
		restart_failed = 1;

	errno = saved_errno;
}

#ifdef ENABLE_COVERAGE_GCOV
extern void __gcov_flush(void);
#endif

static void ATTRIBUTE_NORETURN
terminate(void)
{
	cleanup();
	fflush(NULL);
	if (shared_log != stderr)
		fclose(shared_log);
	if (popen_pid) {
		while (waitpid(popen_pid, NULL, 0) < 0 && errno == EINTR)
			;
	}
	if (exit_code > 0xff) {
		/* Avoid potential core file clobbering.  */
		struct_rlimit rlim = {0, 0};
		set_rlimit(RLIMIT_CORE, &rlim);

		/* Child was killed by a signal, mimic that.  */
		exit_code &= 0xff;
		signal(exit_code, SIG_DFL);
#ifdef ENABLE_COVERAGE_GCOV
		__gcov_flush();
#endif
		raise(exit_code);

		/* Unblock the signal.  */
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, exit_code);
#ifdef ENABLE_COVERAGE_GCOV
		__gcov_flush();
#endif
		sigprocmask(SIG_UNBLOCK, &mask, NULL);

		/* Paranoia - what if this signal is not fatal?
		   Exit with 128 + signo then.  */
		exit_code += 128;
	}
	exit(exit_code);
}

int
main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");
	init(argc, argv);

	exit_code = !nprocs;

	while (dispatch_event(next_event()))
		;
	terminate();
}
