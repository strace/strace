/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <stdarg.h>
#include <limits.h>
#include <fcntl.h>
#include "ptrace.h"
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>
#ifdef HAVE_PATHS_H
# include <paths.h>
#endif
#include <getopt.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <locale.h>
#include <sys/utsname.h>
#include <sys/prctl.h>

#include "kill_save_errno.h"
#include "exitkill.h"
#include "filter_seccomp.h"
#include "largefile_wrappers.h"
#include "mmap_cache.h"
#include "number_set.h"
#include "ptrace_syscall_info.h"
#include "scno.h"
#include "printsiginfo.h"
#include "strauss.h"
#include "trace_event.h"
#include "xstring.h"
#include "delay.h"
#include "wait.h"
#include "secontext.h"

/* In some libc, these aren't declared. Do it ourself: */
extern char **environ;
extern int optind;
extern char *optarg;

#ifdef ENABLE_STACKTRACE
/* if this is true do the stack trace for every system call */
enum stack_trace_modes stack_trace_mode;
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
bool followfork;
bool output_separately;
static unsigned int ptrace_setoptions =
	PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT;
static const struct xlat_data xflag_str[] = {
	{ HEXSTR_NONE,			"none" },
	{ HEXSTR_NON_ASCII_CHARS,	"non-ascii-chars" },
	{ HEXSTR_NON_ASCII,		"non-ascii" },
	{ HEXSTR_ALL,			"all" },
};
unsigned int xflag;
bool debug_flag;
bool Tflag;
int Tflag_scale = 1000;
int Tflag_width = 6;
bool iflag;
bool count_wallclock;
bool tracing_fds;
long long syscall_limit = -1;
static bool nflag;
static int tflag_scale = 1000000000;
static unsigned tflag_width = 0;
static const char *tflag_format = NULL;
static bool rflag;
static int rflag_scale = 1000;
static int rflag_width = 6;
static bool print_pid_pfx;

static unsigned int version_verbosity;

/* -I n */
enum {
	INTR_NOT_SET        = 0,
	INTR_ANYWHERE       = 1, /* don't block/ignore any signals */
	INTR_WHILE_WAIT     = 2, /* block fatal signals while decoding syscall. default */
	INTR_NEVER          = 3, /* block fatal signals. default if '-o FILE PROG' */
	INTR_BLOCK_TSTP_TOO = 4, /* block fatal signals and SIGTSTP (^Z); default if -D */
	NUM_INTR_OPTS
};
static int opt_intr;
/* We play with signal mask only if this mode is active: */
#define interactive (opt_intr == INTR_WHILE_WAIT)

enum {
	DAEMONIZE_NONE        = 0,
	DAEMONIZE_GRANDCHILD  = 1,
	DAEMONIZE_NEW_PGROUP  = 2,
	DAEMONIZE_NEW_SESSION = 3,

	DAEMONIZE_OPTS_GUARD__,
	MAX_DAEMONIZE_OPTS    = DAEMONIZE_OPTS_GUARD__ - 1
};
static const struct xlat_data daemonize_str[] = {
	{ DAEMONIZE_GRANDCHILD,		"grandchild" },
	{ DAEMONIZE_NEW_PGROUP,		"pgroup" },
	{ DAEMONIZE_NEW_PGROUP,		"pgrp" },
	{ DAEMONIZE_NEW_SESSION,	"session" },
};
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
static unsigned int daemonized_tracer;

static int post_attach_sigstop = TCB_IGNORE_ONE_SIGSTOP;
#define use_seize (post_attach_sigstop == 0)

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
	unsigned long msg;   /**< Value returned by PTRACE_GETEVENTMSG */
	siginfo_t si;        /**< siginfo, returned by PTRACE_GETSIGINFO */
};

static struct tcb **tcbtab;
static unsigned int nprocs;
static size_t tcbtabsize;

static struct tcb_wait_data *tcb_wait_tab;
static size_t tcb_wait_tab_size;


#ifndef HAVE_PROGRAM_INVOCATION_NAME
char *program_invocation_name;
#endif

char *argv0; /* override argv[0] on execve */

unsigned os_release; /* generated from uname()'s u.release */

static void detach(struct tcb *tcp);
static void cleanup(int sig);
static void interrupt(int sig);

#ifdef HAVE_SIG_ATOMIC_T
static volatile sig_atomic_t interrupted, restart_failed;
#else
static volatile int interrupted, restart_failed;
#endif

static sigset_t timer_set;
static void timer_sighandler(int);

#ifndef HAVE_STRERROR

# if !HAVE_DECL_SYS_ERRLIST
extern int sys_nerr;
extern char *sys_errlist[];
# endif

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
print_version(unsigned int verbosity)
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
#ifdef ENABLE_SECONTEXT
		" secontext"
#endif
		"";

	printf("%s -- version %s\n"
	       "Copyright (c) 1991-%s The strace developers <%s>.\n"
	       "This is free software; see the source for copying conditions.  There is NO\n"
	       "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	       PACKAGE_NAME, PACKAGE_VERSION, COPYRIGHT_YEAR, PACKAGE_URL);
	printf("\nOptional features enabled:%s\n",
	       features[0] ? features : " (none)");

	/* Raise strauss awareness */
	print_strauss(verbosity);
}

static void
usage(void)
{
#ifdef ENABLE_STACKTRACE
# ifdef USE_LIBDW
#  define K_OPT "kk"
# else
#  define K_OPT "k"
# endif
#else
# define K_OPT ""
#endif
#ifdef ENABLE_SECONTEXT
# define SECONTEXT_OPT "              [--secontext[=FORMAT]]\n"
# define SECONTEXT_E_QUAL ", secontext"
#else
# define SECONTEXT_OPT ""
# define SECONTEXT_E_QUAL ""
#endif

	printf("\
Usage: strace [-ACdffhi" K_OPT "qqrtttTvVwxxyyzZ] [-I N] [-b execve] [-e EXPR]...\n\
              [-a COLUMN] [-o FILE] [-s STRSIZE] [-X FORMAT] [-O OVERHEAD]\n\
              [-S SORTBY] [-P PATH]... [-p PID]... [-U COLUMNS] [--seccomp-bpf]\n"\
              SECONTEXT_OPT "\
              { -p PID | [-DDD] [-E VAR=VAL]... [-u USERNAME] PROG [ARGS] }\n\
   or: strace -c[dfwzZ] [-I N] [-b execve] [-e EXPR]... [-O OVERHEAD]\n\
              [-S SORTBY] [-P PATH]... [-p PID]... [-U COLUMNS] [--seccomp-bpf]\n\
              { -p PID | [-DDD] [-E VAR=VAL]... [-u USERNAME] PROG [ARGS] }\n\
\n\
General:\n\
  -e EXPR        a qualifying expression: OPTION=[!]all or OPTION=[!]VAL1[,VAL2]...\n\
     options:    trace, abbrev, verbose, raw, signal, read, write, fault,\n\
                 inject, status, quiet, kvm, decode-fds" SECONTEXT_E_QUAL "\n\
\n\
Startup:\n\
  -E VAR=VAL, --env=VAR=VAL\n\
                 put VAR=VAL in the environment for command\n\
  -E VAR, --env=VAR\n\
                 remove VAR from the environment for command\n\
  -p PID, --attach=PID\n\
                 trace process with process id PID, may be repeated\n\
  -u USERNAME, --user=USERNAME\n\
                 run command as USERNAME handling setuid and/or setgid\n\
  --argv0=NAME   set PROG argv[0] to NAME\n\
\n\
Tracing:\n\
  -b execve, --detach-on=execve\n\
                 detach on execve syscall\n\
  -D, --daemonize[=grandchild]\n\
                 run tracer process as a grandchild, not as a parent\n\
  -DD, --daemonize=pgroup\n\
                 run tracer process in a separate process group\n\
  -DDD, --daemonize=session\n\
                 run tracer process in a separate session\n\
  -f, --follow-forks\n\
                 follow forks\n\
  -ff, --follow-forks --output-separately\n\
                 follow forks with output into separate files\n\
  -I INTERRUPTIBLE, --interruptible=INTERRUPTIBLE\n\
     1, anywhere:   no signals are blocked\n\
     2, waiting:    fatal signals are blocked while decoding syscall (default)\n\
     3, never:      fatal signals are always blocked (default if '-o FILE PROG')\n\
     4, never_tstp: fatal signals and SIGTSTP (^Z) are always blocked\n\
                    (useful to make 'strace -o FILE PROG' not stop on ^Z)\n\
  --kill-on-exit kill all tracees if strace is killed\n\
\n\
Filtering:\n\
  -e trace=[!][?]{{SYSCALL|GROUP|all|/REGEX}[@64|@32|@x32]|none},\n\
   --trace=[!][?]{{SYSCALL|GROUP|all|/REGEX}[@64|@32|@x32]|none}\n\
                 trace only specified syscalls.\n\
     groups:     %%clock, %%creds, %%desc, %%file, %%fstat, %%fstatfs %%ipc, %%lstat,\n\
                 %%memory, %%net, %%process, %%pure, %%signal, %%stat, %%%%stat,\n\
                 %%statfs, %%%%statfs\n\
  -e signal=SET, --signal=SET\n\
                 trace only the specified set of signals\n\
                 print only the signals from SET\n\
  -e status=SET, --status=SET\n\
                 print only system calls with the return statuses in SET\n\
     statuses:   successful, failed, unfinished, unavailable, detached\n\
  -e trace-fds=SET, --trace-fds=SET\n\
                 trace operations on file descriptors from SET\n\
  -P PATH, --trace-path=PATH\n\
                 trace accesses to PATH\n\
  -z, --successful-only\n\
                 print only syscalls that returned without an error code\n\
  -Z, --failed-only\n\
                 print only syscalls that returned with an error code\n\
\n\
Output format:\n\
  -a COLUMN, --columns=COLUMN\n\
                 alignment COLUMN for printing syscall results (default %d)\n\
  -e abbrev=SET, --abbrev=SET\n\
                 abbreviate output for the syscalls in SET\n\
  -e verbose=SET, --verbose=SET\n\
                 dereference structures for the syscall in SET\n\
  -e raw=SET, --raw=SET\n\
                 print undecoded arguments for the syscalls in SET\n\
  -e read=SET, --read=SET\n\
                 dump the data read from the file descriptors in SET\n\
  -e write=SET, --write=SET\n\
                 dump the data written to the file descriptors in SET\n\
  -e quiet=SET, --quiet=SET\n\
                 suppress various informational messages\n\
     messages:   attach, exit, path-resolution, personality, thread-execve\n\
  -e kvm=vcpu, --kvm=vcpu\n\
                 print exit reason of kvm vcpu\n\
  -e decode-fds=SET, --decode-fds=SET\n\
                 what kinds of file descriptor information details to decode\n\
     details:    dev (device major/minor for block/char device files)\n\
                 path (file path),\n\
                 pidfd (associated PID for pidfds),\n\
                 socket (protocol-specific information for socket descriptors),\n\
                 signalfd (signal masks for signalfds)\n\
"
#ifdef ENABLE_SECONTEXT
"\
  -e secontext=FORMAT, --secontext[=FORMAT]\n\
                 print SELinux contexts in square brackets\n\
     formats:    comma-separated list of all, full, mismatch, none\n\
                 all: equivalent to full,mismatch\n\
                 full: print the full context instead of the type only\n\
                 mismatch: print expected context when actual is not matching\n\
                 none: equivalent to not specifying the option at all\n\
"
#endif
"\
  -i, --instruction-pointer\n\
                 print instruction pointer at time of syscall\n\
"
#ifdef ENABLE_STACKTRACE
"\
  -k, --stack-trace[=symbol]\n\
                 obtain stack trace between each syscall\n\
"
# ifdef USE_LIBDW
"\
  -kk, --stack-trace=source\n\
                 obtain stack trace and source info between each syscall\n\
"
# endif
"\
  --stack-trace-frame-limit=limit\n\
                 obtain no more than this amount of frames\n\
                 when backtracing a syscall (default %d)\n\
"
#endif
"\
  -n, --syscall-number\n\
                 print syscall number\n\
  -o FILE, --output=FILE\n\
                 send trace output to FILE instead of stderr\n\
  -A, --output-append-mode\n\
                 open the file provided in the -o option in append mode\n\
  --output-separately\n\
                 output into separate files (by appending pid to file names)\n\
  -q, --quiet=attach,personality\n\
                 suppress messages about attaching, detaching, etc.\n\
  -qq, --quiet=attach,personality,exit\n\
                 suppress messages about process exit status as well.\n\
  -qqq, --quiet=all\n\
                 suppress all suppressible messages.\n\
  -r, --relative-timestamps[=PRECISION]\n\
                 print relative timestamp\n\
     precision:  one of s, ms, us, ns; default is microseconds\n\
  -s STRSIZE, --string-limit=STRSIZE\n\
                 limit length of print strings to STRSIZE chars (default %d)\n\
  --absolute-timestamps=[[format:]FORMAT[,[precision:]PRECISION]]\n\
                 set the format of absolute timestamps\n\
     format:     none, time, or unix; default is time\n\
     precision:  one of s, ms, us, ns; default is seconds\n\
  -t, --absolute-timestamps[=time]\n\
                 print absolute timestamp\n\
  -tt, --absolute-timestamps=[time,]us\n\
                 print absolute timestamp with usecs\n\
  -ttt, --absolute-timestamps=unix,us\n\
                 print absolute UNIX time with usecs\n\
  -T, --syscall-times[=PRECISION]\n\
                 print time spent in each syscall\n\
     precision:  one of s, ms, us, ns; default is microseconds\n\
  -v, --no-abbrev\n\
                 verbose mode: print entities unabbreviated\n\
  --strings-in-hex=non-ascii-chars\n\
                 use hex instead of octal in escape sequences\n\
  -x, --strings-in-hex=non-ascii\n\
                 print non-ASCII strings in hex\n\
  -xx, --strings-in-hex[=all]\n\
                 print all strings in hex\n\
  -X FORMAT, --const-print-style=FORMAT\n\
                 set the FORMAT for printing of named constants and flags\n\
     formats:    raw, abbrev, verbose\n\
  -y, --decode-fds[=path]\n\
                 print paths associated with file descriptor arguments\n\
  -yy, --decode-fds=all\n\
                 print all available information associated with file\n\
                 descriptors in addition to paths\n\
  --decode-pids=pidns\n\
                 print PIDs in strace's namespace, too\n\
  -Y, --decode-pids=comm\n\
                 print command names associated with PIDs\n\
\n\
Statistics:\n\
  -c, --summary-only\n\
                 count time, calls, and errors for each syscall and report\n\
                 summary\n\
  -C, --summary  like -c, but also print the regular output\n\
  -O OVERHEAD[UNIT], --summary-syscall-overhead=OVERHEAD[UNIT]\n\
                 set overhead for tracing syscalls to OVERHEAD UNITs\n\
     units:      one of s, ms, us, ns; default is microseconds\n\
  -S SORTBY, --summary-sort-by=SORTBY\n\
                 sort syscall counts by: time, min-time, max-time, avg-time,\n\
                 calls, errors, name, nothing (default %s)\n\
  -U COLUMNS, --summary-columns=COLUMNS\n\
                 show specific columns in the summary report: comma-separated\n\
                 list of time-percent, total-time, min-time, max-time, \n\
                 avg-time, calls, errors, name\n\
                 (default time-percent,total-time,avg-time,calls,errors,name)\n\
  -w, --summary-wall-clock\n\
                 summarise syscall latency (default is system time)\n\
\n\
Stop condition:\n\
  --syscall-limit=LIMIT\n\
                 Detach all tracees after tracing LIMIT syscalls\n\
\n\
Tampering:\n\
  -e inject=SET[:error=ERRNO|:retval=VALUE][:signal=SIG][:syscall=SYSCALL]\n\
            [:delay_enter=DELAY][:delay_exit=DELAY]\n\
            [:poke_enter=@argN=DATAN,@argM=DATAM...]\n\
            [:poke_exit=@argN=DATAN,@argM=DATAM...]\n\
            [:when=WHEN],\n\
  --inject=SET[:error=ERRNO|:retval=VALUE][:signal=SIG][:syscall=SYSCALL]\n\
           [:delay_enter=DELAY][:delay_exit=DELAY]\n\
           [:poke_enter=@argN=DATAN,@argM=DATAM...]\n\
           [:poke_exit=@argN=DATAN,@argM=DATAM...]\n\
           [:when=WHEN],\n\
                 perform syscall tampering for the syscalls in SET\n\
     delay:      microseconds or NUMBER{s|ms|us|ns}\n\
     when:       FIRST[..LAST][+[STEP]]\n\
  -e fault=SET[:error=ERRNO][:when=WHEN], --fault=SET[:error=ERRNO][:when=WHEN]\n\
                 synonym for -e inject with default ERRNO set to ENOSYS.\n\
\n\
Miscellaneous:\n\
  -d, --debug    enable debug output to stderr\n\
  -h, --help     print help message\n\
  --seccomp-bpf  enable seccomp-bpf filtering\n\
  --tips[=[[id:]ID][,[format:]FORMAT]]\n\
                 show strace tips, tricks, and tweaks on exit\n\
     id:         non-negative integer or random; default is random\n\
     format:     none, compact, full; default is compact\n\
  -V, --version  print version\n\
"
/* ancient, no one should use it
-F -- attempt to follow vforks (deprecated, use -f)\n\
 */
, DEFAULT_ACOLUMN,
#ifdef ENABLE_STACKTRACE
 DEFAULT_STACK_TRACE_FRAME_LIMIT,
#endif
 DEFAULT_STRLEN, DEFAULT_SORTBY);
	exit(0);

#undef K_OPT
}

void ATTRIBUTE_NORETURN
die(void)
{
	if (strace_tracer_pid == getpid()) {
		cleanup(0);
		exit(1);
	}

	_exit(1);
}

static void
error_opt_arg(int opt, const struct option *lopt, const char *arg)
{
	if (lopt && lopt->name) {
		error_msg_and_help("invalid --%s argument: '%s'",
				   lopt->name, arg);
	} else {
		error_msg_and_help("invalid -%c argument: '%s'", opt, arg);
	}
}

static int
ptrace_attach_or_seize(int pid, const char **ptrace_attach_cmd)
{
	int r;
	if (!use_seize)
		return *ptrace_attach_cmd = "PTRACE_ATTACH",
		       ptrace(PTRACE_ATTACH, pid, 0L, 0L);
	r = ptrace(PTRACE_SEIZE, pid, 0L, (unsigned long) ptrace_setoptions);
	if (r)
		return *ptrace_attach_cmd = "PTRACE_SEIZE", r;
	r = ptrace(PTRACE_INTERRUPT, pid, 0L, 0L);
	return *ptrace_attach_cmd = "PTRACE_INTERRUPT", r;
}

static const char *
ptrace_op_str(unsigned int op)
{
	const char *str = xlookup(ptrace_cmds, op);
	if (str)
		return str;

	static char buf[sizeof(op) * 3];
	xsprintf(buf, "%u", op);
	return buf;
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

	errno = 0;
	ptrace(op, tcp->pid, 0L, (unsigned long) sig);
	err = errno;
	if (!err || err == ESRCH)
		return 0;

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
		tprint_space();
		tprintf_string("<Cannot restart pid %d with ptrace(%s): %s>",
			       tcp->pid, ptrace_op_str(op), strerror(err));
		tprint_newline();
		line_ended();
	}
	errno = err;
	perror_msg("ptrace(%s,pid:%d,sig:%u)",
		   ptrace_op_str(op), tcp->pid, sig);
	return -1;
}

static void
set_cloexec_flag(int fd)
{
	int flags, newflags;

	flags = fcntl_fd(fd, F_GETFD);
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

	if (fcntl_fd(fd, F_SETFD, newflags)) /* never fails */
		perror_msg_and_die("fcntl(%d, F_SETFD, %#x)", fd, newflags);
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
	if (output_separately)
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
tprintf_string(const char *fmt, ...)
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
tprints_string(const char *str)
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
	if (str && *str) {
		tprint_comment_begin();
		tprints_string(str);
		tprint_comment_end();
	}
}

void
tprintf_comment(const char *fmt, ...)
{
	if (!fmt || !*fmt)
		return;

	va_list args;
	va_start(args, fmt);
	tprint_comment_begin();
	tvprintf(fmt, args);
	tprint_comment_end();
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

static void
set_current_tcp(const struct tcb *tcp)
{
	current_tcp = (struct tcb *) tcp;

	/* Sync current_personality and stuff */
	if (current_tcp)
		set_personality(current_tcp->currpers);
}

static void
print_comm_str(const char *str, const size_t len)
{
	if (!len)
		return;
	tprint_associated_info_begin();
	print_quoted_string_ex(str, len,
			       QUOTE_OMIT_LEADING_TRAILING_QUOTES, "<>");
	tprint_associated_info_end();
}

void
printleader(struct tcb *tcp)
{
	/* If -ff, "previous tcb we printed" is always the same as current,
	 * because we have per-tcb output files.
	 */
	if (output_separately)
		printing_tcp = tcp;

	if (printing_tcp) {
		if (printing_tcp->curcol != 0 &&
		    (printing_tcp == tcp ||
		     (!output_separately &&
		      !printing_tcp->staged_output_data))) {
			/*
			 * case 1: we have a shared log (i.e. not -ff), and last line
			 * wasn't finished (same or different tcb, doesn't matter).
			 * case 2: split log, we are the same tcb, but our last line
			 * didn't finish ("SIGKILL nuked us after syscall entry" etc).
			 */
			set_current_tcp(printing_tcp);
			tprint_space();
			tprints_string("<unfinished ...>");
			tprint_newline();
			printing_tcp->curcol = 0;
		}
	}

	printing_tcp = tcp;
	set_current_tcp(tcp);
	current_tcp->curcol = 0;

	if (print_pid_pfx || (nprocs > 1 && !outfname)) {
		size_t len = is_number_in_set(DECODE_PID_COMM, decode_pid_set)
			     ? strlen(tcp->comm) : 0;

		if (print_pid_pfx) {
			if (len)
				tprintf_string("%u", tcp->pid);
			else
				tprintf_string("%-5u", tcp->pid);
		} else {
			tprint_attribute_begin();
			tprintf_string("pid %5u", tcp->pid);
		}

		print_comm_str(tcp->comm, len);

		if (!print_pid_pfx)
			tprint_attribute_end();
		tprint_space();
	}

	selinux_printpidcon(tcp);

	if (tflag_format) {
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);

		time_t local = ts.tv_sec;
		char str[MAX(sizeof("HH:MM:SS"), sizeof(local) * 3)];
		struct tm *tm = localtime(&local);

		if (tm)
			strftime(str, sizeof(str), tflag_format, tm);
		else
			xsprintf(str, "%lld", (long long) local);
		if (tflag_width)
			tprintf_string("%s.%0*ld ", str, tflag_width,
				       (long) ts.tv_nsec / tflag_scale);
		else
			tprintf_string("%s ", str);
	}

	if (rflag) {
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);

		static struct timespec ots;
		if (ots.tv_sec == 0 && ots.tv_nsec == 0)
			ots = ts;

		struct timespec dts;
		ts_sub(&dts, &ts, &ots);
		ots = ts;

		tprintf_string("%s%6ld",
			       tflag_format ? "(+" : "", (long) dts.tv_sec);
		if (rflag_width) {
			tprintf_string(".%0*ld", rflag_width,
				       (long) dts.tv_nsec / rflag_scale);
		}
		tprints_string(tflag_format ? ") " : " ");
	}

	if (nflag)
		print_syscall_number(tcp);

	if (iflag)
		print_instruction_pointer(tcp);
}

void
tabto(void)
{
	if (current_tcp->curcol < acolumn)
		tprints_string(acolumn_spaces + current_tcp->curcol);
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
	if (output_separately) {
		char name[PATH_MAX];
		xsprintf(name, "%s.%u", outfname, tcp->pid);
		tcp->outf = strace_fopen(name);
	}

#ifdef ENABLE_STACKTRACE
	if (stack_trace_mode)
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

	old_tcbtabsize = tcbtabsize;

	tcbtab = xgrowarray(tcbtab, &tcbtabsize, sizeof(tcbtab[0]));
	newtcbs = xcalloc(tcbtabsize - old_tcbtabsize, sizeof(newtcbs[0]));

	for (struct tcb **tcb_ptr = tcbtab + old_tcbtabsize;
	     tcb_ptr < tcbtab + tcbtabsize;
	     ++tcb_ptr, ++newtcbs)
		*tcb_ptr = newtcbs;
}

static char *
strip_trailing_newlines(char *str)
{
	size_t len = strlen(str);
	for (; len > 0; --len) {
		if (str[len - 1] != '\n')
			break;
	}
	str[len] = '\0';
	return str;
}

/* Load the contents of /proc/$pid/comm into `buf'. */
static void
load_pid_comm(int pid, char *buf, size_t buf_size)
{
	static const char comm_path[] = "/proc/%d/comm";
	char procfile[sizeof(comm_path) + sizeof(int) * 3];

	buf[0] = '\0';
	xsprintf(procfile, comm_path, pid);
	FILE *fp = fopen_stream(procfile, "r");
	if (fp) {
		if (fgets(buf, buf_size, fp))
			strip_trailing_newlines(buf);
		fclose(fp);
	}
}

void
print_pid_comm(int pid)
{
	char buf[PROC_COMM_LEN];
	load_pid_comm(pid, buf, sizeof(buf));
	print_comm_str(buf, strlen(buf));
}

void
maybe_load_task_comm(struct tcb *tcp)
{
	if (!is_number_in_set(DECODE_PID_COMM, decode_pid_set))
		return;

	load_pid_comm(get_proc_pid(tcp->pid), tcp->comm, sizeof(tcp->comm));
}

static struct tcb *
alloctcb(int pid)
{
	if (nprocs == tcbtabsize)
		expand_tcbtab();

	for (size_t i = 0; i < tcbtabsize; ++i) {
		struct tcb *tcp = tcbtab[i];
		if (!tcp->pid) {
			memset(tcp, 0, sizeof(*tcp));
			list_init(&tcp->wait_list);
			tcp->pid = pid;
			maybe_load_task_comm(tcp);
#if SUPPORTED_PERSONALITIES > 1
			tcp->currpers = current_personality;
#endif
#ifdef ENABLE_SECONTEXT
			tcp->last_dirfd = AT_FDCWD;
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

	if (cflag && debug_flag) {
		struct timespec dt;

		ts_sub(&dt, &tcp->stime, &tcp->atime);
		debug_func_msg("pid %d: %.9f seconds of system time spent "
			       "since attach", tcp->pid, ts_float(&dt));
	}

	for (int p = 0; p < SUPPORTED_PERSONALITIES; ++p)
		free(tcp->inject_vec[p]);

	free_tcb_priv_data(tcp);

#ifdef ENABLE_STACKTRACE
	if (stack_trace_mode)
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
		bool publish = true;
		if (!is_complete_set(status_set, NUMBER_OF_STATUSES)) {
			publish = is_number_in_set(STATUS_DETACHED, status_set);
			strace_close_memstream(tcp, publish);
		}

		if (output_separately) {
			if (tcp->curcol != 0 && publish)
				fprintf(tcp->outf, " <detached ...>\n");
			fclose(tcp->outf);
		} else {
			if (printing_tcp == tcp && tcp->curcol != 0 && publish)
				fprintf(tcp->outf, " <detached ...>\n");
			flush_tcp_output(tcp);
		}
	}

	if (current_tcp == tcp)
		set_current_tcp(NULL);
	if (printing_tcp == tcp)
		printing_tcp = NULL;

	list_remove(&tcp->wait_list);

	memset(tcp, 0, sizeof(*tcp));
}

static void
droptcb_verbose(struct tcb *tcp)
{
	if (!is_number_in_set(QUIET_ATTACH, quiet_set)
	    && (tcp->flags & TCB_ATTACHED))
		error_msg("Process %u detached", tcp->pid);

	droptcb(tcp);
}

/* Returns true when the tracee has to be waited for. */
static bool
interrupt_or_stop(struct tcb *tcp)
{
	/*
	 * Linux wrongly insists the child be stopped
	 * before detaching.  Arghh.  We go through hoops
	 * to make a clean break of things.
	 */

	if (!(tcp->flags & TCB_ATTACHED))
		return false;

	/* We attached but possibly didn't see the expected SIGSTOP.
	 * We must catch exactly one as otherwise the detached process
	 * would be left stopped (process state T).
	 */
	if (tcp->flags & TCB_IGNORE_ONE_SIGSTOP)
		return true;

	int error = ptrace(PTRACE_DETACH, tcp->pid, 0, 0);
	if (!error) {
		/* On a clear day, you can see forever. */
		return false;
	}
	if (errno != ESRCH) {
		/* Shouldn't happen. */
		perror_func_msg("ptrace(PTRACE_DETACH,%u)", tcp->pid);
		return false;
	}
	/* ESRCH: process is either not stopped or doesn't exist. */
	if (my_tkill(tcp->pid, 0) < 0) {
		if (errno != ESRCH)
			/* Shouldn't happen. */
			perror_func_msg("tkill(%u,0)", tcp->pid);
		/* else: process doesn't exist. */
		return false;
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
			return true;
		if (errno != ESRCH)
			perror_func_msg("ptrace(PTRACE_INTERRUPT,%u)", tcp->pid);
	} else {
		error = my_tkill(tcp->pid, SIGSTOP);
		if (!error)
			return true;
		if (errno != ESRCH)
			perror_func_msg("tkill(%u,SIGSTOP)", tcp->pid);
	}

	/* Either process doesn't exist, or some weird error. */
	return false;
}

/* Returns true if the tracee can be passed to droptcb. */
static bool
detach_interrupted_or_stopped(struct tcb *tcp, int status)
{
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
		return true;
	}
	unsigned int sig = WSTOPSIG(status);
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
		return true;
	}
	/* Note: this check has to be after use_seize check */
	/* (else, in use_seize case SIGSTOP will be mistreated) */
	if (sig == SIGSTOP) {
		/* Detach, suppressing SIGSTOP */
		ptrace_restart(PTRACE_DETACH, tcp, 0);
		return true;
	}
	if (sig == syscall_trap_sig)
		sig = 0;
	/* Can't detach just yet, may need to wait for SIGSTOP */
	int error = ptrace_restart(PTRACE_CONT, tcp, sig);
	if (error < 0) {
		/* Should not happen.
		 * Note: ptrace_restart returns 0 on ESRCH, so it's not it.
		 * ptrace_restart already emitted error message.
		 */
		return true;
	}
	return false;
}

/* Detach traced process.
 * Never call DETACH twice on the same process as both unattached and
 * attached-unstopped processes give the same ESRCH.  For unattached process we
 * would SIGSTOP it and wait for its SIGSTOP notification forever.
 */
static void
detach(struct tcb *tcp)
{
	if (!interrupt_or_stop(tcp))
		goto drop;

	/*
	 * We end up here in three cases:
	 * 1. We sent PTRACE_INTERRUPT (use_seize case)
	 * 2. We sent SIGSTOP (!use_seize)
	 * 3. Attach SIGSTOP was already pending (TCB_IGNORE_ONE_SIGSTOP set)
	 */
	for (;;) {
		int status;
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
		if (detach_interrupted_or_stopped(tcp, status))
			break;
	}

 drop:
	droptcb_verbose(tcp);
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
		size_t adv = strcspn(opt, "\n\t ,");
		if (adv == 0) {
			opt++;
			continue;
		}
		char *delim = opt + adv;
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
	const char *ptrace_attach_cmd;

	if (ptrace_attach_or_seize(tcp->pid, &ptrace_attach_cmd) < 0) {
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
	    xsprintf(procdir, task_path, get_proc_pid(tcp->pid)) > 0 &&
	    (dir = opendir(procdir)) != NULL) {
		struct_dirent *de;

		while ((de = read_dir(dir)) != NULL) {
			if (de->d_fileno == 0)
				continue;

			int tid = string_to_uint(de->d_name);
			if (tid <= 0 || tid == tcp->pid)
				continue;

			++ntid;
			if (ptrace_attach_or_seize(tid, &ptrace_attach_cmd) < 0)
			{
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

	if (!is_number_in_set(QUIET_ATTACH, quiet_set)) {
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

		switch (daemonized_tracer) {
		case DAEMONIZE_NEW_PGROUP:
			/*
			 * If -D is passed twice, create a new process group,
			 * so we won't be killed by kill(0, ...).
			 */
			if (setpgid(0, 0) < 0)
				perror_msg_and_die("Cannot create a new"
						   " process group");
			break;
		case DAEMONIZE_NEW_SESSION:
			/*
			 * If -D is passed thrice, create a new session,
			 * so we won't be killed upon session termination.
			 */
			if (setsid() < 0)
				perror_msg_and_die("Cannot create a new"
						   " session");
			break;
		}

		debug_msg("new tracer pid is %d", strace_tracer_pid);
	}

	for (size_t tcbi = 0; tcbi < tcbtabsize; ++tcbi) {
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

static void
maybe_init_seccomp_filter(void)
{
	debug_msg("seccomp filter %s",
		  seccomp_filtering ? "enabled" : "disabled");
	if (seccomp_filtering)
		init_seccomp_filter();
}

/* Stack-o-phobic exec helper, in the hope to work around
 * NOMMU + "daemonized tracer" difficulty.
 */
struct exec_params {
	int fd_to_close;
	uid_t run_euid;
	gid_t run_egid;
	char **argv;
	char **env;
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

	if (!daemonized_tracer) {
		if (params->child_sa.sa_handler != SIG_DFL)
			sigaction(SIGCHLD, &params->child_sa, NULL);

		if (!use_seize && ptrace(PTRACE_TRACEME, 0L, 0L, 0L) < 0)
			perror_msg_and_die("ptrace(PTRACE_TRACEME, ...)");
	}

	if (username != NULL) {
		/*
		 * It is important to set groups before we
		 * lose privileges on setuid.
		 */
		if (initgroups(username, run_gid) < 0)
			perror_msg_and_die("initgroups");
		if (setregid(run_gid, params->run_egid) < 0)
			perror_msg_and_die("setregid");

		/*
		 * If there is a seccomp filter to be installed, this should
		 * be done before CAP_SYS_ADMIN is dropped by setreuid.
		 */
		maybe_init_seccomp_filter();

		if (setreuid(run_uid, params->run_euid) < 0)
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
		while (wait(NULL) < 0 && errno == EINTR)
			;
		alarm(0);
		if (params->child_sa.sa_handler != SIG_DFL)
			sigaction(SIGCHLD, &params->child_sa, NULL);
	}

	if (!username)
		maybe_init_seccomp_filter();
	execve(params->pathname, params->argv, params->env);
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
	/*
	 * It might be a good idea to redirect stderr as well,
	 * but we sometimes need to print error messages.
	 */
	for (int i = 0; i <= 1; ++i) {
		if (!fd_is_placeholder[i]) {
			close(i);
			open_dummy_desc();
		}
	}
}

static void
startup_child(char **argv, char **env)
{
	strace_stat_t statbuf;
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
	} else {
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
	if (filename && !*pathname)
		error_msg_and_die("Cannot find executable '%s'", filename);
	if (stat_file(pathname, &statbuf) < 0)
		perror_msg_and_die("Cannot stat '%s'", pathname);

	params_for_tracee.fd_to_close = (shared_log != stderr) ? fileno(shared_log) : -1;
	params_for_tracee.run_euid = (statbuf.st_mode & S_ISUID) ? statbuf.st_uid : run_uid;
	params_for_tracee.run_egid = (statbuf.st_mode & S_ISGID) ? statbuf.st_gid : run_gid;
	params_for_tracee.argv = argv;
	if (argv0)
		params_for_tracee.argv[0] = argv0;
	params_for_tracee.env = env;
	/*
	 * On NOMMU, can be safely freed only after execve in tracee.
	 * It's hard to know when that happens, so we just leak it.
	 */
	params_for_tracee.pathname = NOMMU_SYSTEM ? xstrdup(pathname) : pathname;

	if (daemonized_tracer)
		prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);

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

			const char *ptrace_attach_cmd;
			if (ptrace_attach_or_seize(pid, &ptrace_attach_cmd)) {
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

	if (seccomp_filtering)
		tcp->flags |= TCB_SECCOMP_FILTER;

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

static unsigned int
get_os_release(void)
{
	struct utsname u;
	if (uname(&u) < 0)
		perror_msg_and_die("uname");
	/*
	 * u.release string consists of at most three parts
	 * and normally has this form: "3.2.9[-some-garbage]",
	 * "X.Y-something" means "X.Y.0".
	 */
	const char *p = u.release;
	unsigned int rel = 0;
	for (unsigned int parts = 0; parts < 3; ++parts) {
		unsigned int n = 0;
		for (; (*p >= '0') && (*p <= '9'); ++p) {
			n *= 10;
			n += *p - '0';
		}
		rel <<= 8;
		rel |= n;
		if (*p == '.')
			++p;
	}
	return rel;
}

static void
set_sighandler(int signo, void (*sighandler)(int), struct sigaction *oldact)
{
	const struct sigaction sa = { .sa_handler = sighandler };
	sigaction(signo, &sa, oldact);
}

static int
parse_interruptible_arg(const char *arg)
{
	static const struct xlat_data intr_str[] = {
		{ INTR_ANYWHERE,	"anywhere" },
		{ INTR_ANYWHERE,	"always" },
		{ INTR_WHILE_WAIT,	"waiting" },
		{ INTR_NEVER,		"never" },
		{ INTR_BLOCK_TSTP_TOO,	"never_tstp" },
	};

	const struct xlat_data *intr_arg = find_xlat_val(intr_str, arg);

	return intr_arg ? (int) intr_arg->val
			: (int) string_to_uint_upto(arg, NUM_INTR_OPTS - 1);
}

static int
parse_ts_arg(const char *in_arg)
{
	static const char format_pfx[] = "format:";
	static const char scale_pfx[] = "precision:";

	enum {
		TOKEN_FORMAT = 1 << 0,
		TOKEN_SCALE  = 1 << 1,
	} token_type;
	enum {
		FK_UNSET,
		FK_NONE,
		FK_TIME,
		FK_UNIX,
	} format_kind = FK_UNSET;
	int precision_width;
	int precision_scale = 0;
	char *arg = xstrdup(in_arg);
	char *saveptr = NULL;

	for (const char *token = strtok_r(arg, ",", &saveptr);
	     token; token = strtok_r(NULL, ",", &saveptr)) {
		token_type = TOKEN_FORMAT | TOKEN_SCALE;

		if (!strncasecmp(token, format_pfx, sizeof(format_pfx) - 1)) {
			token += sizeof(format_pfx) - 1;
			token_type = TOKEN_FORMAT;
		} else if (!strncasecmp(token, scale_pfx,
					sizeof(scale_pfx) - 1)) {
			token += sizeof(scale_pfx) - 1;
			token_type = TOKEN_SCALE;

		}

		if (token_type & TOKEN_FORMAT) {
			if (!strcasecmp(token, "none")) {
				format_kind = FK_NONE;
				continue;
			} else if (!strcasecmp(token, "time")) {
				format_kind = FK_TIME;
				continue;
			} else if (!strcasecmp(token, "unix")) {
				format_kind = FK_UNIX;
				continue;
			}
		}

		if (token_type & TOKEN_SCALE) {
			precision_scale =
				str2timescale_optarg(token, &precision_width);

			if (precision_scale > 0)
				continue;
		}

		free(arg);
		return -1;
	}

	switch (format_kind) {
	case FK_UNSET:
		if (!tflag_format)
			tflag_format = "%T";
		break;
	case FK_NONE:
		tflag_format = NULL;
		break;
	case FK_TIME:
		tflag_format = "%T";
		break;
	case FK_UNIX:
		tflag_format = "%s";
		break;
	}

	if (precision_scale > 0) {
		tflag_scale = precision_scale;
		tflag_width = precision_width;
	}

	free(arg);
	return 0;
}

static int
parse_tips_arg(const char *in_arg)
{
	static const char id_pfx[] = "id:";
	static const char fmt_pfx[] = "format:";

	enum {
		TOKEN_ID = 1 << 0,
		TOKEN_FORMAT = 1 << 1,
	} token_type;
	int id = tip_id;
	enum tips_fmt fmt = show_tips == TIPS_NONE ? TIPS_COMPACT : show_tips;
	char *arg = xstrdup(in_arg);
	char *saveptr = NULL;

	for (const char *token = strtok_r(arg, ",", &saveptr);
	     token; token = strtok_r(NULL, ",", &saveptr)) {
		token_type = TOKEN_ID | TOKEN_FORMAT;

		if (!strncasecmp(token, id_pfx, sizeof(id_pfx) - 1)) {
			token += sizeof(id_pfx) - 1;
			token_type = TOKEN_ID;
		} else if (!strncasecmp(token, fmt_pfx,
					sizeof(fmt_pfx) - 1)) {
			token += sizeof(fmt_pfx) - 1;
			token_type = TOKEN_FORMAT;

		}

		if (token_type & TOKEN_ID) {
			int ret;

			if (!strcasecmp(token, "random")) {
				id = TIP_ID_RANDOM;
				continue;
			} else if ((ret = string_to_uint(token)) >= 0) {
				id = ret;
				continue;
			}
		}

		if (token_type & TOKEN_FORMAT) {
			if (!strcasecmp(token, "none")) {
				fmt = TIPS_NONE;
				continue;
			} else if (!strcasecmp(token, "compact")) {
				fmt = TIPS_COMPACT;
				continue;
			} else if (!strcasecmp(token, "full")) {
				fmt = TIPS_FULL;
				continue;
			}
		}

		free(arg);
		return -1;
	}

	tip_id = id;
	show_tips = fmt;

	free(arg);
	return 0;
}

static void
remove_from_env(char **env, size_t *env_count, const char *var)
{
	const size_t len = strlen(var);
	size_t w = 0;

	debug_func_msg("Removing variable \"%s\" from the command environment",
		       var);

	for (size_t r = 0; r < *env_count; ++r) {
		if (!strncmp(env[r], var, len) &&
		    (env[r][len] == '=' || env[r][len] == '\0')) {
			debug_func_msg("Skipping entry %zu (\"%s\")",
				       r, env[r]);
			continue;
		}
		if (w < r) {
			debug_func_msg("Copying entry %zu to %zu", r, w);
			env[w] = env[r];
		}
		++w;
	}

	if (w < *env_count) {
		debug_func_msg("Decreasing env count from %zu to %zu",
			       *env_count, w);
		*env_count = w;
	}
}

static void
add_to_env(char **env, size_t *env_count, char *var, const size_t len)
{
	size_t r;

	for (r = 0; r < *env_count; ++r) {
		if (!strncmp(env[r], var, len) &&
		    (env[r][len] == '=' || env[r][len] == '\0'))
			break;
	}

	if (r < *env_count) {
		debug_func_msg("Replacing entry %zu (\"%s\")"
			       ", key=\"%.*s\", var=\"%s\"",
			       r, env[r], (int) len, var, var);
	} else {
		debug_func_msg("Adding entry %zu"
			       ", key=\"%.*s\", var=\"%s\"",
			       r, (int) len, var, var);
		*env_count += 1;
	}

	env[r] = var;
}

static void
update_env(char **env, size_t *env_count, char *var)
{
	char *val = strchr(var, '=');

	if (val)
		add_to_env(env, env_count, var, val - var);
	else
		remove_from_env(env, env_count, var);
}

static char **
make_env(char **orig_env, char *const *env_changes, size_t env_change_count)
{
	if (!env_change_count)
		return orig_env;

	char **new_env;
	size_t new_env_count = 0;
	size_t new_env_size;

	/* Determining the environment variable count.  */
	if (orig_env) {
		for (; orig_env[new_env_count]; ++new_env_count)
			;
	}
	new_env_size = new_env_count + env_change_count;

	if (new_env_size < new_env_count || new_env_size < env_change_count ||
	    new_env_size + 1 < new_env_size)
		error_msg_and_die("Cannot construct new environment: the sum "
				  "of old environment variable count (%zu) and "
				  "environment changes count (%zu) is too big",
				  new_env_count, env_change_count);

	new_env_size++;
	new_env = xallocarray(new_env_size, sizeof(*new_env));
	if (new_env_count)
		memcpy(new_env, orig_env, new_env_count * sizeof(*orig_env));

	for (size_t i = 0; i < env_change_count; ++i)
		update_env(new_env, &new_env_count, env_changes[i]);

	new_env[new_env_count] = NULL;

	return new_env;
}

static void
increase_version_verbosity(void)
{
	if (version_verbosity < (STRAUSS_START_VERBOSITY + strauss_lines))
		version_verbosity++;
}

struct pathtrace {
	struct path_set_item *paths;
	size_t size;
	size_t count;
};

static void
add_path_trace(struct pathtrace *pt, const char *path)
{
	if (pt->count >= pt->size) {
		pt->paths = xgrowarray(pt->paths, &pt->size,
				       sizeof(pt->paths[0]));
	}

	pt->paths[pt->count].path = path;
	pt->count++;
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
	static const char qflag_qual[] = "attach,personality";
	static const char qqflag_qual[] = "exit,attach,personality";
	static const char qqqflag_qual[] = "all";
	static const char yflag_qual[] = "path";
	static const char yyflag_qual[] = "all";
	static const char tflag_str[] = "format:time";
	static const char ttflag_str[] = "precision:us,format:time";
	static const char tttflag_str[] = "format:unix,precision:us";
	static const char secontext_qual[] = "!full,mismatch";

	int c, i;
	int optF = 0, zflags = 0;
	int lopt_idx;
	int daemonized_tracer_long = DAEMONIZE_NONE;
	int xflag_long = -1;
	int qflag_short = 0;
	int followfork_short = 0;
	int yflag_short = 0;
	bool tflag_long_set = false;
	int tflag_short = 0;
	bool columns_set = false;
	bool sortby_set = false;
	bool opt_kill_on_exit = false;
#ifdef ENABLE_STACKTRACE
	int stack_trace_frame_limit = 0;
#endif

	/*
	 * We can initialise global_path_set only after tracing backend
	 * initialisation, so we store pointers to all the paths from
	 * command-line arguments during parsing in this array and then,
	 * after the successful backend initialisation, iterate over it
	 * in order to add them to global_path_set.
	 */
	struct pathtrace pathtrace = { NULL };

	/**
	 * Storage for environment changes requested for command.  They
	 * are stored in a temporary array and not applied as is during
	 * command line parsing for two reasons:
	 *  - putenv() changes environment of the tracer as well,
	 *    which is unacceptable.
	 *  - Environment changes have to be applied
	 *    in a tracing-backend-specific way.
	 */
	char **env_changes = NULL;
	size_t env_change_size = 0;
	size_t env_change_count = 0;

	if (!program_invocation_name || !*program_invocation_name) {
		static char name[] = "strace";
		program_invocation_name =
			(argc > 0 && argv[0] && *argv[0]) ? argv[0] : name;
	}

	strace_tracer_pid = getpid();

	os_release = get_os_release();

	pidns_init();

	shared_log = stderr;
	set_sortby(DEFAULT_SORTBY);
	set_personality(DEFAULT_PERSONALITY);
	qualify_trace("all");
	qualify_abbrev("all");
	qualify_verbose("all");
#if DEFAULT_QUAL_FLAGS != (QUAL_TRACE | QUAL_ABBREV | QUAL_VERBOSE)
# error Bug in DEFAULT_QUAL_FLAGS
#endif
	qualify_status("all");
	qualify_quiet("none");
	qualify_decode_fd("none");
	qualify_signals("all");
#ifdef ENABLE_SECONTEXT
	qualify_secontext("none");
#endif

	static const char optstring[] =
		"+a:Ab:cCdDe:E:fFhiI:kno:O:p:P:qrs:S:tTu:U:vVwxX:yYzZ";

	enum {
		GETOPT_SECCOMP = 0x100,
		GETOPT_DAEMONIZE,
		GETOPT_HEX_STR,
		GETOPT_FOLLOWFORKS,
		GETOPT_KILL_ON_EXIT,
		GETOPT_OUTPUT_SEPARATELY,
		GETOPT_PIDNS_TRANSLATION,
		GETOPT_SYSCALL_LIMIT,
		GETOPT_STACK,
		GETOPT_TS,
		GETOPT_TIPS,
		GETOPT_ARGV0,
		GETOPT_STACK_TRACE_FRAME_LIMIT,

		GETOPT_QUAL_TRACE,
		GETOPT_QUAL_TRACE_FD,
		GETOPT_QUAL_ABBREV,
		GETOPT_QUAL_VERBOSE,
		GETOPT_QUAL_RAW,
		GETOPT_QUAL_SIGNAL,
		GETOPT_QUAL_STATUS,
		GETOPT_QUAL_READ,
		GETOPT_QUAL_WRITE,
		GETOPT_QUAL_FAULT,
		GETOPT_QUAL_INJECT,
		GETOPT_QUAL_KVM,
		GETOPT_QUAL_QUIET,
		GETOPT_QUAL_DECODE_FD,
		GETOPT_QUAL_DECODE_PID,
		GETOPT_QUAL_SECONTEXT,
	};
	static const struct option longopts[] = {
		{ "columns",		required_argument, 0, 'a' },
		{ "output-append-mode",	no_argument,	   0, 'A' },
		{ "detach-on",		required_argument, 0, 'b' },
		{ "summary-only",	no_argument,	   0, 'c' },
		{ "summary",		no_argument,	   0, 'C' },
		{ "debug",		no_argument,	   0, 'd' },
		{ "daemonize",		optional_argument, 0, GETOPT_DAEMONIZE },
		{ "daemonised",		optional_argument, 0, GETOPT_DAEMONIZE },
		{ "daemonized",		optional_argument, 0, GETOPT_DAEMONIZE },
		{ "env",		required_argument, 0, 'E' },
		{ "follow-forks",	no_argument,	   0, GETOPT_FOLLOWFORKS },
		{ "output-separately",	no_argument,	   0,
			GETOPT_OUTPUT_SEPARATELY },
		{ "help",		no_argument,	   0, 'h' },
		{ "instruction-pointer", no_argument,      0, 'i' },
		{ "interruptible",	required_argument, 0, 'I' },
		{ "kill-on-exit",	no_argument,	   0, GETOPT_KILL_ON_EXIT },
		{ "stack-trace" ,	optional_argument, 0, GETOPT_STACK },
		{ "stack-traces" ,	optional_argument, 0, GETOPT_STACK },
		{ "stack-trace-frame-limit", required_argument, 0, GETOPT_STACK_TRACE_FRAME_LIMIT },
		{ "syscall-limit",	required_argument, 0, GETOPT_SYSCALL_LIMIT },
		{ "syscall-number",	no_argument,	   0, 'n' },
		{ "output",		required_argument, 0, 'o' },
		{ "summary-syscall-overhead", required_argument, 0, 'O' },
		{ "attach",		required_argument, 0, 'p' },
		{ "trace-path",		required_argument, 0, 'P' },
		{ "relative-timestamps", optional_argument, 0, 'r' },
		{ "string-limit",	required_argument, 0, 's' },
		{ "summary-sort-by",	required_argument, 0, 'S' },
		{ "absolute-timestamps", optional_argument, 0, GETOPT_TS },
		{ "timestamps",		optional_argument, 0, GETOPT_TS },
		{ "syscall-times",	optional_argument, 0, 'T' },
		{ "user",		required_argument, 0, 'u' },
		{ "summary-columns",	required_argument, 0, 'U' },
		{ "no-abbrev",		no_argument,	   0, 'v' },
		{ "version",		no_argument,	   0, 'V' },
		{ "summary-wall-clock", no_argument,	   0, 'w' },
		{ "strings-in-hex",	optional_argument, 0, GETOPT_HEX_STR },
		{ "const-print-style",	required_argument, 0, 'X' },
		{ "pidns-translation",	no_argument      , 0, GETOPT_PIDNS_TRANSLATION },
		{ "successful-only",	no_argument,	   0, 'z' },
		{ "failed-only",	no_argument,	   0, 'Z' },
		{ "failing-only",	no_argument,	   0, 'Z' },
		{ "seccomp-bpf",	no_argument,	   0, GETOPT_SECCOMP },
		{ "tips",		optional_argument, 0, GETOPT_TIPS },
		{ "argv0",		required_argument, 0, GETOPT_ARGV0 },

		{ "trace",	required_argument, 0, GETOPT_QUAL_TRACE },
		{ "trace-fds",	required_argument, 0, GETOPT_QUAL_TRACE_FD },
		{ "abbrev",	required_argument, 0, GETOPT_QUAL_ABBREV },
		{ "verbose",	required_argument, 0, GETOPT_QUAL_VERBOSE },
		{ "raw",	required_argument, 0, GETOPT_QUAL_RAW },
		{ "signals",	required_argument, 0, GETOPT_QUAL_SIGNAL },
		{ "status",	required_argument, 0, GETOPT_QUAL_STATUS },
		{ "read",	required_argument, 0, GETOPT_QUAL_READ },
		{ "write",	required_argument, 0, GETOPT_QUAL_WRITE },
		{ "fault",	required_argument, 0, GETOPT_QUAL_FAULT },
		{ "inject",	required_argument, 0, GETOPT_QUAL_INJECT },
		{ "kvm",	required_argument, 0, GETOPT_QUAL_KVM },
		{ "quiet",	optional_argument, 0, GETOPT_QUAL_QUIET },
		{ "silent",	optional_argument, 0, GETOPT_QUAL_QUIET },
		{ "silence",	optional_argument, 0, GETOPT_QUAL_QUIET },
		{ "decode-fds",	optional_argument, 0, GETOPT_QUAL_DECODE_FD },
		{ "decode-pids",required_argument, 0, GETOPT_QUAL_DECODE_PID },
		{ "secontext",	optional_argument, 0, GETOPT_QUAL_SECONTEXT },

		{ 0, 0, 0, 0 }
	};

	lopt_idx = -1;
	while ((c = getopt_long(argc, argv, optstring, longopts, &lopt_idx)) != EOF) {
		const struct option *lopt = lopt_idx >= 0
			&& (unsigned) lopt_idx < ARRAY_SIZE(longopts)
			? longopts + lopt_idx : NULL;
		lopt_idx = -1;

		switch (c) {
		case 'a':
			acolumn = string_to_uint(optarg);
			if (acolumn < 0)
				error_opt_arg(c, lopt, optarg);
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
				error_msg_and_help("-c/--summary-only and "
						   "-C/--summary are mutually "
						   "exclusive");
			}
			cflag = CFLAG_ONLY_STATS;
			break;
		case 'C':
			if (cflag == CFLAG_ONLY_STATS) {
				error_msg_and_help("-c/--summary-only and "
						   "-C/--summary are mutually "
						   "exclusive");
			}
			cflag = CFLAG_BOTH;
			break;
		case 'd':
			debug_flag = 1;
			break;
		case 'D':
			daemonized_tracer++;
			break;
		case GETOPT_DAEMONIZE:
			daemonized_tracer_long =
				find_arg_val(optarg, daemonize_str,
					     DAEMONIZE_GRANDCHILD,
					     DAEMONIZE_NONE);
			if (daemonized_tracer_long <= DAEMONIZE_NONE)
				error_opt_arg(c, lopt, optarg);
			break;
		case 'e':
			qualify(optarg);
			break;
		case 'E':
			if (env_change_count >= env_change_size)
				env_changes = xgrowarray(env_changes,
							 &env_change_size,
							 sizeof(*env_changes));

			env_changes[env_change_count++] = optarg;
			break;
		case 'f':
			followfork_short++;
			break;
		case GETOPT_FOLLOWFORKS:
			followfork = true;
			break;
		case GETOPT_OUTPUT_SEPARATELY:
			output_separately = true;
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
			opt_intr = parse_interruptible_arg(optarg);
			if (opt_intr <= 0)
				error_opt_arg(c, lopt, optarg);
			break;
		case 'k':
#ifdef ENABLE_STACKTRACE
			switch (stack_trace_mode) {
			case STACK_TRACE_OFF:
				stack_trace_mode = STACK_TRACE_ON;
				break;
			case STACK_TRACE_ON:
# ifdef USE_LIBDW
				stack_trace_mode = STACK_TRACE_WITH_SRCINFO;
# else
				error_msg_and_die("Stack traces with "
						  "source line information (-kk/"
						  "--stack-trace=source option) "
						  "are not supported by this "
						  "build of strace");
# endif	/* USE_LIBDW */
				break;
			default:
				error_msg_and_die("Too many -k options");
			}
#else
			error_msg_and_die("Stack traces (-k/--stack-trace "
					  "option) are not supported by this "
					  "build of strace");
#endif /* ENABLE_STACKTRACE */
			break;
		case GETOPT_STACK:
#ifdef ENABLE_STACKTRACE
			if (optarg == NULL || strcmp(optarg, "symbol") == 0)
				stack_trace_mode = STACK_TRACE_ON;
			else if (strcmp(optarg, "source") == 0) {
# ifdef USE_LIBDW
				stack_trace_mode = STACK_TRACE_WITH_SRCINFO;
# else
				error_msg_and_die("Stack traces with "
						  "source line information "
						  "(-kk/--stack-trace=source option) "
						  "are not supported by this "
						  "build of strace");
# endif /* USE_LIBDW */
			} else
				error_opt_arg(c, lopt, optarg);
#else
			error_msg_and_die("Stack traces (-k/--stack-trace "
					  "option) are not supported by this "
					  "build of strace");
#endif /* ENABLE_STACKTRACE */
			break;
		case GETOPT_STACK_TRACE_FRAME_LIMIT:
			i = string_to_uint(optarg);
			if (i <= 0 || (unsigned int) i > -1U / 4)
				error_opt_arg(c, lopt, optarg);
#ifdef ENABLE_STACKTRACE
			stack_trace_frame_limit = i;
#else
			error_msg_and_die("Stack traces "
					  "(--stack-trace-frame-limit "
					  "option) are not supported "
					  "by this build of strace");
#endif
			break;
		case GETOPT_KILL_ON_EXIT:
			opt_kill_on_exit = true;
			break;
		case 'n':
			nflag = 1;
			break;
		case 'o':
			outfname = optarg;
			break;
		case 'O':
			if (set_overhead(optarg) < 0)
				error_opt_arg(c, lopt, optarg);
			break;
		case 'p':
			process_opt_p_list(optarg);
			break;
		case 'P':
			add_path_trace(&pathtrace, optarg);
			break;
		case 'q':
			qflag_short++;
			break;
		case 'r':
			rflag = 1;
			rflag_width = 6;
			rflag_scale = str2timescale_optarg(optarg,
							   &rflag_width);
			if (rflag_scale < 0)
				error_opt_arg(c, lopt, optarg);
			break;
		case 's':
			i = string_to_uint(optarg);
			if (i < 0 || (unsigned int) i > -1U / 4)
				error_opt_arg(c, lopt, optarg);
			max_strlen = i;
			break;
		case 'S':
			set_sortby(optarg);
			sortby_set = true;
			break;
		case 't':
			tflag_short++;
			break;
		case GETOPT_TS:
			tflag_long_set = true;
			if (parse_ts_arg(optarg ?: tflag_str))
				error_opt_arg(c, lopt, optarg);
			break;
		case 'T':
			Tflag = 1;
			Tflag_width = 6;
			Tflag_scale = str2timescale_optarg(optarg,
							   &Tflag_width);
			if (Tflag_scale < 0)
				error_opt_arg(c, lopt, optarg);
			break;
		case 'u':
			username = optarg;
			break;
		case 'U':
			columns_set = true;
			set_count_summary_columns(optarg);
			break;
		case 'v':
			qualify_abbrev("none");
			break;
		case 'V':
			increase_version_verbosity();
			break;
		case 'w':
			count_wallclock = 1;
			break;
		case 'x':
			xflag = MIN(xflag + 1, HEXSTR_ALL);
			break;
		case GETOPT_HEX_STR:
			xflag_long = find_arg_val(optarg, xflag_str,
						  HEXSTR_ALL, -1);
			if (xflag_long < HEXSTR_NONE)
				error_opt_arg(c, lopt, optarg);
			break;
		case 'X':
			if (!strcmp(optarg, "raw"))
				xlat_verbosity = XLAT_STYLE_RAW;
			else if (!strcmp(optarg, "abbrev"))
				xlat_verbosity = XLAT_STYLE_ABBREV;
			else if (!strcmp(optarg, "verbose"))
				xlat_verbosity = XLAT_STYLE_VERBOSE;
			else
				error_opt_arg(c, lopt, optarg);
			break;
		case 'y':
			yflag_short++;
			break;
		case 'Y':
			qualify_decode_pid("comm");
			break;
		case GETOPT_PIDNS_TRANSLATION:
			qualify_decode_pid("pidns");
			break;
		case 'z':
			clear_number_set_array(status_set, 1);
			add_number_to_set(STATUS_SUCCESSFUL, status_set);
			zflags++;
			break;
		case 'Z':
			clear_number_set_array(status_set, 1);
			add_number_to_set(STATUS_FAILED, status_set);
			zflags++;
			break;
		case GETOPT_SECCOMP:
			seccomp_filtering = true;
			break;
		case GETOPT_SYSCALL_LIMIT:
			syscall_limit = string_to_ulonglong(optarg);
			if (syscall_limit <= 0)
				error_opt_arg(c, lopt, optarg);
			break;
		case GETOPT_TIPS:
			if (parse_tips_arg(optarg ?: ""))
				error_opt_arg(c, lopt, optarg);
			break;
		case GETOPT_ARGV0:
			argv0 = optarg;
			break;
		case GETOPT_QUAL_SECONTEXT:
			qualify_secontext(optarg ? optarg : secontext_qual);
			break;
		case GETOPT_QUAL_TRACE:
			qualify_trace(optarg);
			break;
		case GETOPT_QUAL_TRACE_FD:
			qualify_trace_fd(optarg);
			break;
		case GETOPT_QUAL_ABBREV:
			qualify_abbrev(optarg);
			break;
		case GETOPT_QUAL_VERBOSE:
			qualify_verbose(optarg);
			break;
		case GETOPT_QUAL_RAW:
			qualify_raw(optarg);
			break;
		case GETOPT_QUAL_SIGNAL:
			qualify_signals(optarg);
			break;
		case GETOPT_QUAL_STATUS:
			qualify_status(optarg);
			break;
		case GETOPT_QUAL_READ:
			qualify_read(optarg);
			break;
		case GETOPT_QUAL_WRITE:
			qualify_write(optarg);
			break;
		case GETOPT_QUAL_FAULT:
			qualify_fault(optarg);
			break;
		case GETOPT_QUAL_INJECT:
			qualify_inject(optarg);
			break;
		case GETOPT_QUAL_KVM:
			qualify_kvm(optarg);
			break;
		case GETOPT_QUAL_QUIET:
			qualify_quiet(optarg ?: qflag_qual);
			break;
		case GETOPT_QUAL_DECODE_FD:
			qualify_decode_fd(optarg ?: yflag_qual);
			break;
		case GETOPT_QUAL_DECODE_PID:
			qualify_decode_pid(optarg);
			break;
		default:
			error_msg_and_help(NULL);
			break;
		}
	}

	if (version_verbosity) {
		print_version(version_verbosity);
		exit(0);
	}

	argv += optind;
	argc -= optind;

	if (argc < 0 || (!nprocs && !argc)) {
		if (show_tips != TIPS_NONE) {
			print_totd();
			exit(exit_code);
		}
		error_msg_and_help("must have PROG [ARGS] or -p PID");
	}

	if (!argc && argv0)
		error_msg_and_help("PROG [ARGS] must be specified with --argv0");

	if (daemonized_tracer_long) {
		if (daemonized_tracer) {
			error_msg_and_die("-D and --daemonize cannot"
					  " be provided simultaneously");
		} else {
			daemonized_tracer = daemonized_tracer_long;
		}
	}

	if (!argc && daemonized_tracer) {
		error_msg_and_help("PROG [ARGS] must be specified with "
				   "-D/--daemonize");
	}

	if (daemonized_tracer > (unsigned int) MAX_DAEMONIZE_OPTS)
		error_msg_and_help("Too many -D's (%u), maximum supported -D "
				   "count is %d",
				   daemonized_tracer, MAX_DAEMONIZE_OPTS);

	if (tflag_short) {
		if (tflag_long_set) {
			error_msg_and_die("-t and --absolute-timestamps cannot"
					  " be provided simultaneously");
		}

		parse_ts_arg(tflag_short == 1 ? tflag_str :
			     tflag_short == 2 ? ttflag_str : tttflag_str);
	}

	if (xflag_long >= 0) {
		if (xflag) {
			error_msg_and_die("-x and --strings-in-hex cannot"
					  " be provided simultaneously");
		} else {
			xflag = xflag_long;
		}
	}

	if (yflag_short) {
		if (decode_fd_set_updated) {
			error_msg_and_die("-y and --decode-fds cannot"
					  " be provided simultaneously");
		}

		qualify_decode_fd(yflag_short == 1 ? yflag_qual : yyflag_qual);
	}

	if (is_number_in_set(DECODE_PID_COMM, decode_pid_set)) {
		/*
		 * If --decode-pids=comm option comes after -p, comm fields
		 * of tcbs are not filled though tcbs are initialized.
		 * We must fill the fields here.
		 */
		for (size_t i = 0; i < tcbtabsize; ++i) {
			struct tcb *tcp = tcbtab[i];
			if (tcp->comm[0] == 0)
				maybe_load_task_comm(tcp);
		}
	}

	if (followfork_short) {
		if (followfork) {
			error_msg_and_die("-f and --follow-forks cannot"
					  " be provided simultaneously");
		} else if (followfork_short >= 2 && output_separately) {
			error_msg_and_die("-ff and --output-separately cannot"
					  " be provided simultaneously");
		} else {
			followfork = true;
			output_separately = followfork_short >= 2;
		}
	}

	if (seccomp_filtering && !followfork) {
		error_msg("--seccomp-bpf cannot be used without"
			  " -f/--follow-forks, disabling");
		seccomp_filtering = false;
	}

	if (seccomp_filtering && detach_on_execve) {
		error_msg("--seccomp-bpf is not enabled because"
			  " it is not compatible with -b");
		seccomp_filtering = false;
	}

	if (seccomp_filtering && syscall_limit > 0) {
		error_msg("--seccomp-bpf is not enabled because"
			  " it is not compatible with --syscall-limit");
		seccomp_filtering = false;
	}

	if (seccomp_filtering && nprocs) {
		error_msg("--seccomp-bpf is not enabled for processes"
			  " attached with -p");
		if (argc == 0)
			seccomp_filtering = false;
	}

	if (optF) {
		if (followfork) {
			error_msg("deprecated option -F ignored");
		} else {
			error_msg("option -F is deprecated, "
				  "please use -f/--follow-forks instead");
			followfork = true;
		}
	}

	if (output_separately && cflag) {
		error_msg_and_help("(-c/--summary-only or -C/--summary) and"
				   " -ff/--output-separately"
				   " are mutually exclusive");
	}

	if (count_wallclock && !cflag) {
		error_msg_and_help("-w/--summary-wall-clock must be given with"
				   " (-c/--summary-only or -C/--summary)");
	}

	if (columns_set && !cflag) {
		error_msg_and_help("-U/--summary-columns must be given with"
				   " (-c/--summary-only or -C/--summary)");
	}

	if (sortby_set && !cflag) {
		error_msg("-S/--summary-sort-by has no effect without"
			  " (-c/--summary-only or -C/--summary)");
	}

	if (cflag == CFLAG_ONLY_STATS) {
		if (iflag)
			error_msg("-i/--instruction-pointer has no effect "
				  "with -c/--summary-only");
		if (stack_trace_mode)
			error_msg("-k/--stack-trace has no effect "
				  "with -c/--summary-only");
		if (nflag)
			error_msg("-n/--syscall-number has no effect "
				  "with -c/--summary-only");
		if (rflag)
			error_msg("-r/--relative-timestamps has no effect "
				  "with -c/--summary-only");
		if (tflag_format)
			error_msg("-t/--absolute-timestamps has no effect "
				  "with -c/--summary-only");
		if (Tflag)
			error_msg("-T/--syscall-times has no effect "
				  "with -c/--summary-only");
		if (!number_set_array_is_empty(decode_fd_set, 0))
			error_msg("-y/--decode-fds has no effect "
				  "with -c/--summary-only");
#ifdef ENABLE_SECONTEXT
		if (!number_set_array_is_empty(secontext_set, 0))
			error_msg("--secontext has no effect with "
				  "-c/--summary-only");
#endif
	}

	if (!outfname) {
		if (output_separately && !followfork)
			error_msg("--output-separately has no effect "
				  "without -o/--output");
		if (open_append)
			error_msg("-A/--output-append-mode has no effect "
				  "without -o/--output");
	}

#ifndef HAVE_OPEN_MEMSTREAM
	if (!is_complete_set(status_set, NUMBER_OF_STATUSES))
		error_msg_and_help("open_memstream is required to use -z, -Z, or -e status");
#endif

	if (zflags > 1)
		error_msg("Only the last of "
			  "-z/--successful-only/-Z/--failed-only options will "
			  "take effect. "
			  "See status qualifier for more complex filters.");

	for (size_t cnt = 0; cnt < pathtrace.count; ++cnt)
		pathtrace_select(pathtrace.paths[cnt].path);
	free(pathtrace.paths);

	acolumn_spaces = xmalloc(acolumn + 1);
	memset(acolumn_spaces, ' ', acolumn);
	acolumn_spaces[acolumn] = '\0';

	set_sighandler(SIGCHLD, SIG_DFL, &params_for_tracee.child_sa);

#ifdef ENABLE_STACKTRACE
	if (stack_trace_mode) {
		if (stack_trace_frame_limit == 0)
			stack_trace_frame_limit =
				DEFAULT_STACK_TRACE_FRAME_LIMIT;
		unwind_init(stack_trace_mode == STACK_TRACE_WITH_SRCINFO,
			    stack_trace_frame_limit);
	} else if (stack_trace_frame_limit != 0) {
		error_msg("--stack-trace-frame-limit has no effect "
			  "without -k/--stack-trace");
	}
#endif

	/* See if they want to run as another user. */
	if (username != NULL) {
		struct passwd *pent;

		if (getuid() != 0 || geteuid() != 0) {
			error_msg_and_die("You must be root to use "
					  "the -u/--username option");
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

	if (seccomp_filtering)
		check_seccomp_filter();
	if (seccomp_filtering) {
		ptrace_setoptions |= PTRACE_O_TRACESECCOMP;
		if (nprocs == 0 && is_exitkill_supported())
			ptrace_setoptions |= PTRACE_O_EXITKILL;
	}

	if (opt_kill_on_exit) {
		if (nprocs)
			error_msg_and_die("--kill-on-exit and -p/--attach"
					  " are mutually exclusive options");
		if (!is_exitkill_supported())
			error_msg_and_die("PTRACE_O_EXITKILL is not supported"
					  " by the kernel");
		ptrace_setoptions |= PTRACE_O_EXITKILL;
	}

	debug_msg("ptrace_setoptions = %#x", ptrace_setoptions);
	test_ptrace_seize();
	test_ptrace_get_syscall_info();

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
			if (output_separately)
				error_msg_and_help("piping the output and "
						   "-ff/--output-separately "
						   "are mutually exclusive");
			shared_log = strace_popen(outfname + 1);
		} else if (!output_separately) {
			shared_log = strace_fopen(outfname);
		} else if (strlen(outfname) >= PATH_MAX - sizeof(int) * 3) {
			errno = ENAMETOOLONG;
			perror_msg_and_die("%s", outfname);
		}
	} else {
		/* -ff without -o FILE is the same as single -f */
		output_separately = false;
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

	if (daemonized_tracer && !opt_intr)
		opt_intr = INTR_BLOCK_TSTP_TOO;
	if (outfname && argc) {
		if (!opt_intr)
			opt_intr = INTR_NEVER;
		if (!qflag_short && !quiet_set_updated)
			qflag_short = 1;
	}
	if (!opt_intr)
		opt_intr = INTR_WHILE_WAIT;

	if (qflag_short) {
		if (quiet_set_updated) {
			error_msg_and_die("-q and -e quiet/--quiet cannot"
					  " be provided simultaneously");
		}

		qualify_quiet(qflag_short == 1 ? qflag_qual :
			      qflag_short == 2 ? qqflag_qual : qqqflag_qual);
	}

	/*
	 * startup_child() must be called before the signal handlers get
	 * installed below as they are inherited into the spawned process.
	 * Also we do not need to be protected by them as during interruption
	 * in the startup_child() mode we kill the spawned process anyway.
	 */
	if (argc) {
		char **new_environ = make_env(environ, env_changes,
					      env_change_count);
		free(env_changes);

		startup_child(argv, new_environ);

		/*
		 * On a NOMMU system, new_environ can be freed only after exec
		 * in child, so we leak it in that case, similar to pathname
		 * in startup_child().
		 */
		if (new_environ != environ && !NOMMU_SYSTEM)
			free(new_environ);
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
	print_pid_pfx = outfname && !output_separately &&
		(followfork || nprocs > 1);
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

	for (size_t i = 0; i < tcbtabsize; ++i) {
		tcp = tcbtab[i];
		if (tcp->pid == pid)
			return *ptcp = tcp;
	}

	return NULL;
}

static const char *
status2str(int status)
{
	if (WIFEXITED(status))
		return "Exit";
	if (WIFSIGNALED(status))
		return "Termination";
	if (WIFSTOPPED(status))
		return "Stop";
	return "Wait event";
}

static void
print_debug_info(const int pid, int status)
{
	const unsigned int event = (unsigned int) status >> 16;
	char buf[sizeof("WIFEXITED,exitcode=%u") + sizeof(int)*3 /*paranoia:*/ + 16];
	char evbuf[sizeof(",EVENT_VFORK_DONE (%u)") + sizeof(int)*3 /*paranoia:*/ + 16];

	strcpy(buf, "???");
	if (WIFSIGNALED(status))
		xsprintf(buf, "WIFSIGNALED,%ssig=%s",
				WCOREDUMP(status) ? "core," : "",
				sprintsigname(WTERMSIG(status)));
	if (WIFEXITED(status))
		xsprintf(buf, "WIFEXITED,exitcode=%u", WEXITSTATUS(status));
	if (WIFSTOPPED(status))
		xsprintf(buf, "WIFSTOPPED,sig=%s",
			 sprintsigname(WSTOPSIG(status)));
	evbuf[0] = '\0';
	if (event != 0) {
		static const char *const event_names[] = {
			[PTRACE_EVENT_CLONE] = "CLONE",
			[PTRACE_EVENT_FORK]  = "FORK",
			[PTRACE_EVENT_VFORK] = "VFORK",
			[PTRACE_EVENT_VFORK_DONE] = "VFORK_DONE",
			[PTRACE_EVENT_EXEC]  = "EXEC",
			[PTRACE_EVENT_EXIT]  = "EXIT",
			[PTRACE_EVENT_SECCOMP]  = "SECCOMP",
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

static void
cleanup(int fatal_sig)
{
	if (ptrace_setoptions & PTRACE_O_EXITKILL)
		return;

	if (!fatal_sig)
		fatal_sig = SIGTERM;

	size_t num_to_wait = 0;

	for (size_t i = 0; i < tcbtabsize; ++i) {
		struct tcb *tcp = tcbtab[i];
		if (!tcp->pid)
			continue;
		debug_func_msg("looking at pid %u", tcp->pid);
		if (tcp->pid == strace_child) {
			kill(tcp->pid, SIGCONT);
			kill(tcp->pid, fatal_sig);
		}
		if (interrupt_or_stop(tcp))
			++num_to_wait;
		else
			droptcb_verbose(tcp);
	}

	while (num_to_wait) {
		int status;
		pid_t pid = waitpid(-1, &status, __WALL);

		if (pid < 0) {
			if (errno == EINTR)
				continue;
			/* ECHILD is not expected */
			perror_func_msg("waitpid(-1, __WALL)");
			break;
		}

		if (pid == popen_pid) {
			if (!WIFSTOPPED(status))
				popen_pid = 0;
			continue;
		}

		if (debug_flag)
			print_debug_info(pid, status);

		struct tcb *tcp = pid2tcb(pid);
		if (!tcp) {
			if (!is_number_in_set(QUIET_EXIT, quiet_set)) {
				/*
				 * This can happen if we inherited an unknown child.
				 */
				error_msg("%s of unknown pid %u ignored",
					  status2str(status), pid);
			}
			continue;
		}

		if (detach_interrupted_or_stopped(tcp, status)) {
			droptcb_verbose(tcp);
			--num_to_wait;
		}
	}
}

static void
interrupt(int sig)
{
	interrupted = sig;
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
		if (!is_number_in_set(QUIET_EXIT, quiet_set)) {
			/*
			 * This can happen if we inherited an unknown child.
			 * Example: (sleep 1 & exec strace true)
			 */
			error_msg("%s of unknown pid %u ignored",
				  status2str(status), pid);
		}
		return NULL;
	}
	if (followfork) {
		/* We assume it's a fork/vfork/clone child */
		struct tcb *tcp = alloctcb(pid);
		after_successful_attach(tcp, post_attach_sigstop);
		if (!is_number_in_set(QUIET_ATTACH, quiet_set))
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
		if (!is_number_in_set(QUIET_ATTACH, quiet_set))
			error_msg("Detached unknown pid %d", pid);
		return NULL;
	}
}

/*
 * Under Linux, execve changes pid to thread leader's pid, and we see this
 * changed pid on EVENT_EXEC and later, execve sysexit.  Leader "disappears"
 * without exit notification.  Let user know that, drop leader's tcb, and fix
 * up pid in execve thread's tcb.  Effectively, execve thread's tcb replaces
 * leader's tcb.
 *
 * BTW, leader is 'stuck undead' (doesn't report WIFEXITED on exit syscall)
 * in multi-threaded programs exactly in order to handle this case.
 */
static struct tcb *
maybe_switch_tcbs(struct tcb *tcp, const int pid)
{
	/*
	 * PTRACE_GETEVENTMSG returns old pid starting from Linux 3.0.
	 * On 2.6 and earlier it can return garbage.
	 */
	if (os_release < KERNEL_VERSION(3, 0, 0))
		return NULL;

	const long old_pid = tcb_wait_tab[tcp->wait_data_idx].msg;

	/* Avoid truncation in pid2tcb() param passing */
	if (old_pid <= 0 || old_pid == pid)
		return NULL;
	if ((unsigned long) old_pid > UINT_MAX)
		return NULL;
	struct tcb *execve_thread = pid2tcb(old_pid);
	/* It should be !NULL, but I feel paranoid */
	if (!execve_thread)
		return NULL;

	if (execve_thread->curcol != 0) {
		/*
		 * One case we are here is -ff, try
		 * "strace -oLOG -ff test/threaded_execve".
		 * Another case is demonstrated by
		 * tests/maybe_switch_current_tcp.c
		 */
		fprintf(execve_thread->outf, " <pid changed to %d ...>\n", pid);
		/*execve_thread->curcol = 0; - no need, see code below */
	}
	/* Swap output FILEs and memstream (needed for -ff) */
	FILE *fp = execve_thread->outf;
	execve_thread->outf = tcp->outf;
	tcp->outf = fp;
	if (execve_thread->staged_output_data || tcp->staged_output_data) {
		struct staged_output_data *staged_output_data;

		staged_output_data = execve_thread->staged_output_data;
		execve_thread->staged_output_data = tcp->staged_output_data;
		tcp->staged_output_data = staged_output_data;
	}

	/* And their column positions */
	execve_thread->curcol = tcp->curcol;
	tcp->curcol = 0;
	/* Drop leader, but close execve'd thread outfile (if -ff) */
	droptcb(tcp);
	/* Switch to the thread, reusing leader's outfile and pid */
	tcp = execve_thread;
	tcp->pid = pid;
	if (cflag != CFLAG_ONLY_STATS) {
		if (!is_number_in_set(QUIET_THREAD_EXECVE, quiet_set)) {
			printleader(tcp);
			tprintf_string("+++ superseded by execve in pid %lu +++",
				       old_pid);
			tprint_newline();
			line_ended();
		}
		/*
		 * Need to reopen memstream for thread
		 * as we closed it in droptcb.
		 */
		if (!is_complete_set(status_set, NUMBER_OF_STATUSES))
			strace_open_memstream(tcp);
		tcp->flags |= TCB_REPRINT;
	}

	return tcp;
}

static struct tcb *
maybe_switch_current_tcp(void)
{
	struct tcb *tcp = maybe_switch_tcbs(current_tcp, current_tcp->pid);

	if (tcp)
		set_current_tcp(tcp);

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
		tprintf_string("+++ killed by %s %s+++",
			       sprintsigname(WTERMSIG(status)),
			       WCOREDUMP(status) ? "(core dumped) " : "");
		tprint_newline();
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
	    !is_number_in_set(QUIET_EXIT, quiet_set)) {
		printleader(tcp);
		tprintf_string("+++ exited with %d +++", WEXITSTATUS(status));
		tprint_newline();
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
			tprintf_string("--- %s ", sprintsigname(sig));
			printsiginfo(tcp, si);
			tprints_string(" ---");
		} else
			tprintf_string("--- stopped by %s ---", sprintsigname(sig));
		tprint_newline();
		line_ended();

#ifdef ENABLE_STACKTRACE
		if (stack_trace_mode)
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

	if (cflag) {
		tcp->atime = tcp->stime;
	}
}

static void
print_event_exit(struct tcb *tcp)
{
	if (entering(tcp) || filtered(tcp) || hide_log(tcp)
	    || cflag == CFLAG_ONLY_STATS) {
		return;
	}

	if (!output_separately && printing_tcp && printing_tcp != tcp
	    && printing_tcp->curcol != 0 && !printing_tcp->staged_output_data) {
		set_current_tcp(printing_tcp);
		tprint_space();
		tprints_string("<unfinished ...>");
		tprint_newline();
		flush_tcp_output(printing_tcp);
		printing_tcp->curcol = 0;
		set_current_tcp(tcp);
	}

	print_syscall_resume(tcp);

	if (!(tcp->sys_func_rval & RVAL_DECODED)) {
		/*
		 * The decoder has probably decided to print something
		 * on exiting syscall which is not going to happen.
		 */
		tprint_space();
		tprints_string("<unfinished ...>");
	}

	tprints_string(") ");
	tabto();
	tprint_sysret_begin();
	tprints_sysret_next("retval");
	tprint_sysret_pseudo_rval();
	tprint_sysret_end();
	tprint_newline();
	if (!is_complete_set(status_set, NUMBER_OF_STATUSES)) {
		bool publish = is_number_in_set(STATUS_UNFINISHED, status_set);
		strace_close_memstream(tcp, publish);
	}
	line_ended();
}

static size_t
trace_wait_data_size(struct tcb *tcp)
{
	return sizeof(struct tcb_wait_data);
}

static struct tcb_wait_data *
init_trace_wait_data(void *p)
{
	struct tcb_wait_data *wd = p;

	memset(wd, 0, sizeof(*wd));

	return wd;
}

static struct tcb_wait_data *
copy_trace_wait_data(const struct tcb_wait_data *wd)
{
	return xobjdup(wd);
}

static void
free_trace_wait_data(struct tcb_wait_data *wd)
{
	free(wd);
}

static void
tcb_wait_tab_check_size(const size_t size)
{
	while (size >= tcb_wait_tab_size) {
		tcb_wait_tab = xgrowarray(tcb_wait_tab,
					  &tcb_wait_tab_size,
					  sizeof(tcb_wait_tab[0]));
	}
}

static const struct tcb_wait_data *
next_event(void)
{
	if (interrupted)
		return NULL;

	if (syscall_limit == 0) {
		if (!is_number_in_set(QUIET_ATTACH, quiet_set))
			error_msg("System call limit has been reached, detaching tracees");
		strace_child = 0;
		return NULL;
	}

	invalidate_umove_cache();

	struct tcb *tcp = NULL;
	struct list_item *elem;

	static EMPTY_LIST(pending_tcps);
	/* Handle the queued tcbs before waiting for new events.  */
	if (!list_is_empty(&pending_tcps))
		goto next_event_get_tcp;

	static struct tcb *extra_tcp;
	static size_t wait_extra_data_idx;
	/* Handle the extra tcb event.  */
	if (extra_tcp) {
		tcp = extra_tcp;
		extra_tcp = NULL;
		tcp->wait_data_idx = wait_extra_data_idx;

		debug_msg("dequeued extra event for pid %u", tcp->pid);
		goto next_event_exit;
	}

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
	int status;
	struct rusage ru;
	int pid = wait4(-1, &status, __WALL, (cflag ? &ru : NULL));
	int wait_errno = errno;

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

	size_t wait_tab_pos = 0;
	bool wait_nohang = false;

	/*
	 * Wait for new events until wait4() returns 0 (meaning that there's
	 * nothing more to wait for for now), or a second event for some tcb
	 * appears (which may happen if a tracee was SIGKILL'ed, for example).
	 */
	for (;;) {
		struct tcb_wait_data *wd;

		if (pid < 0) {
			if (wait_errno == EINTR)
				break;
			if (wait_nohang)
				break;
			if (nprocs == 0 && wait_errno == ECHILD)
				return NULL;
			/*
			 * If nprocs > 0, ECHILD is not expected,
			 * treat it as any other error here:
			 */
			errno = wait_errno;
			perror_msg_and_die("wait4(__WALL)");
		}

		if (!pid)
			break;

		if (pid == popen_pid) {
			if (!WIFSTOPPED(status))
				popen_pid = 0;
			break;
		}

		if (debug_flag)
			print_debug_info(pid, status);

		/* Look up 'pid' in our table. */
		tcp = pid2tcb(pid);

		if (!tcp) {
			tcp = maybe_allocate_tcb(pid, status);
			if (!tcp)
				goto next_event_wait_next;
		}

		if (cflag) {
			tcp->stime.tv_sec = ru.ru_stime.tv_sec;
			tcp->stime.tv_nsec = ru.ru_stime.tv_usec * 1000;
		}

		tcb_wait_tab_check_size(wait_tab_pos);

		/* Initialise a new wait data structure.  */
		wd = tcb_wait_tab + wait_tab_pos;
		init_trace_wait_data(wd);
		wd->status = status;

		if (WIFSIGNALED(status)) {
			wd->te = TE_SIGNALLED;
		} else if (WIFEXITED(status)) {
			wd->te = TE_EXITED;
		} else {
			/*
			 * As WCONTINUED flag has not been specified to wait4,
			 * it cannot be WIFCONTINUED(status), so the only case
			 * that remains is WIFSTOPPED(status).
			 */

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
				if (sig == SIGSTOP &&
				    (tcp->flags & TCB_IGNORE_ONE_SIGSTOP)) {
					debug_func_msg("ignored SIGSTOP on "
						       "pid %d", tcp->pid);
					tcp->flags &= ~TCB_IGNORE_ONE_SIGSTOP;
					wd->te = TE_RESTART;
				} else if (sig == syscall_trap_sig) {
					wd->te = TE_SYSCALL_STOP;
				} else {
					/*
					 * True if tracee is stopped by signal
					 * (as opposed to "tracee received
					 * signal").
					 * TODO: shouldn't we check for
					 * errno == EINVAL too?
					 * We can get ESRCH instead, you know...
					 */
					bool stopped = ptrace(PTRACE_GETSIGINFO,
						pid, 0, &wd->si) < 0;

					wd->te = stopped ? TE_GROUP_STOP
							 : TE_SIGNAL_DELIVERY_STOP;
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
					/*
					 * TODO: shouldn't we check for
					 * errno == EINVAL here, too?
					 * We can get ESRCH instead, you know...
					 */
				if (ptrace(PTRACE_GETEVENTMSG, pid, NULL,
				    &wd->msg) < 0)
					wd->msg = 0;

				wd->te = TE_STOP_BEFORE_EXECVE;
				break;
			case PTRACE_EVENT_EXIT:
				wd->te = TE_STOP_BEFORE_EXIT;
				break;
			case PTRACE_EVENT_SECCOMP:
				wd->te = TE_SECCOMP;
				break;
			default:
				wd->te = TE_RESTART;
			}
		}

		if (!wd->te)
			error_func_msg("Tracing event hasn't been determined "
				       "for pid %d, status %0#x", pid, status);

		if (!list_is_empty(&tcp->wait_list)) {
			wait_extra_data_idx = wait_tab_pos;
			extra_tcp = tcp;
			debug_func_msg("queued extra pid %d", tcp->pid);
		} else {
			tcp->wait_data_idx = wait_tab_pos;
			list_append(&pending_tcps, &tcp->wait_list);
			debug_func_msg("queued pid %d", tcp->pid);
		}

		wait_tab_pos++;

		if (extra_tcp)
			break;

next_event_wait_next:
		pid = wait4(-1, &status, __WALL | WNOHANG, (cflag ? &ru : NULL));
		wait_errno = errno;
		wait_nohang = true;
	}

next_event_get_tcp:
	elem = list_remove_head(&pending_tcps);

	if (!elem) {
		tcb_wait_tab_check_size(0);
		memset(tcb_wait_tab, 0, sizeof(*tcb_wait_tab));
		tcb_wait_tab->te = TE_NEXT;

		return tcb_wait_tab;
	} else {
		tcp = list_elem(elem, struct tcb, wait_list);
		debug_func_msg("dequeued pid %d", tcp->pid);
	}

next_event_exit:
	/* Is this the very first time we see this tracee stopped? */
	if (tcp->flags & TCB_STARTUP)
		startup_tcb(tcp);

	clear_regs(tcp);

	/* Set current output file */
	set_current_tcp(tcp);

	return tcb_wait_tab + tcp->wait_data_idx;
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
	unsigned int restart_op;
	unsigned int restart_sig = 0;
	enum trace_event te = wd ? wd->te : TE_BREAK;
	/*
	 * Copy wd->status to a non-const variable to workaround glibc bugs
	 * around union wait fixed by glibc commit glibc-2.24~391
	 */
	int status = wd ? wd->status : 0;

	if (current_tcp && has_seccomp_filter(current_tcp))
		restart_op = seccomp_filter_restart_operator(current_tcp);
	else
		restart_op = PTRACE_SYSCALL;

	switch (te) {
	case TE_BREAK:
		return false;

	case TE_NEXT:
		return true;

	case TE_RESTART:
		break;

	case TE_SECCOMP:
		if (!has_seccomp_filter(current_tcp)) {
			/*
			 * We don't know if forks/clones have a seccomp filter
			 * when they are created, but we can detect it when we
			 * have a seccomp-stop.
			 * In such a case, if !seccomp_before_sysentry, we have
			 * already processed the syscall entry, so we avoid
			 * processing it a second time.
			 */
			current_tcp->flags |= TCB_SECCOMP_FILTER;
			restart_op = PTRACE_SYSCALL;
			break;
		}

		if (seccomp_before_sysentry) {
			restart_op = PTRACE_SYSCALL;
			break;
		}
		ATTRIBUTE_FALLTHROUGH;

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
		if (has_seccomp_filter(current_tcp)) {
			/*
			 * Syscall and seccomp stops can happen in different
			 * orders depending on kernel.  strace tests this in
			 * check_seccomp_order_tracer().
			 *
			 * Linux 3.5--4.7:
			 * (seccomp-stop before syscall-entry-stop)
			 *         +--> seccomp-stop ->-PTRACE_SYSCALL->-+
			 *         |                                     |
			 *     PTRACE_CONT                   syscall-entry-stop
			 *         |                                     |
			 * syscall-exit-stop <---PTRACE_SYSCALL-----<----+
			 *
			 * Linux 4.8+:
			 * (seccomp-stop after syscall-entry-stop)
			 *                 syscall-entry-stop
			 *
			 *         +---->-----PTRACE_CONT---->----+
			 *         |                              |
			 *  syscall-exit-stop               seccomp-stop
			 *         |                              |
			 *         +----<----PTRACE_SYSCALL---<---+
			 *
			 * Note in Linux 4.8+, we restart in PTRACE_CONT
			 * after syscall-exit to skip the syscall-entry-stop.
			 * The next seccomp-stop will be treated as a syscall
			 * entry.
			 *
			 * The line below implements this behavior.
			 * Note that exiting(current_tcp) actually marks
			 * a syscall-entry-stop because the flag was inverted
			 * in the above call to trace_syscall.
			 */
			restart_op = exiting(current_tcp) ? PTRACE_SYSCALL : PTRACE_CONT;
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
		/* The syscall succeeded, clear the flag.  */
		current_tcp->flags &= ~TCB_CHECK_EXEC_SYSCALL;
		/*
		 * Check that we are inside syscall now (next event after
		 * PTRACE_EVENT_EXEC should be for syscall exiting).  If it is
		 * not the case, we might have a situation when we attach to a
		 * process and the first thing we see is a PTRACE_EVENT_EXEC
		 * and all the following syscall state tracking is screwed up
		 * otherwise.
		 */
		if (!maybe_switch_current_tcp() && entering(current_tcp)) {
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
	if (syscall_delayed(current_tcp)) {
		if (current_tcp->delayed_wait_data)
			error_func_msg("pid %d has delayed wait data set"
				       " already", current_tcp->pid);

		current_tcp->delayed_wait_data = copy_trace_wait_data(wd);

		return true;
	}

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
	struct tcb_wait_data *wd = tcp->delayed_wait_data;

	if (!wd) {
		error_func_msg("No delayed wait data found for pid %d",
			       tcp->pid);
		wd = init_trace_wait_data(alloca(trace_wait_data_size(tcp)));
	}

	wd->te = TE_RESTART;

	debug_func_msg("pid %d", tcp->pid);

	tcp->flags &= ~TCB_DELAYED;

	struct tcb *const prev_tcp = current_tcp;
	current_tcp = tcp;
	bool ret = dispatch_event(wd);
	current_tcp = prev_tcp;

	free_trace_wait_data(tcp->delayed_wait_data);
	tcp->delayed_wait_data = NULL;

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

static void ATTRIBUTE_NORETURN
terminate(void)
{
	int sig = interrupted;

	cleanup(sig);
	if (cflag)
		call_summary(shared_log);
	fflush(NULL);
	if (shared_log != stderr)
		fclose(shared_log);
	if (popen_pid) {
		while (waitpid(popen_pid, NULL, 0) < 0 && errno == EINTR)
			;
	}
	if (sig) {
		exit_code = 0x100 | sig;
	}
	if (exit_code > 0xff) {
		/* Avoid potential core file clobbering.  */
		struct_rlimit rlim = {0, 0};
		set_rlimit(RLIMIT_CORE, &rlim);

		/* Child was killed by a signal, mimic that.  */
		exit_code &= 0xff;
		signal(exit_code, SIG_DFL);
		print_totd();
		GCOV_DUMP;
		raise(exit_code);

		/* Unblock the signal.  */
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, exit_code);
		GCOV_DUMP;
		sigprocmask(SIG_UNBLOCK, &mask, NULL);

		/* Paranoia - what if this signal is not fatal?
		   Exit with 128 + signo then.  */
		exit_code += 128;
	}

	print_totd();
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
