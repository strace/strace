/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
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

#include <sys/types.h>
#include "defs.h"

#include <signal.h>
#include <errno.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <limits.h>

#if defined(IA64) && defined(LINUX)
# include <asm/ptrace_offsets.h>
#endif

#ifdef USE_PROCFS
#include <poll.h>
#endif

#ifdef SVR4
#include <sys/stropts.h>
#ifdef HAVE_MP_PROCFS
#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif
#endif
#endif

int debug = 0, followfork = 0, followvfork = 0, interactive = 0;
int rflag = 0, tflag = 0, dtime = 0, cflag = 0;
int iflag = 0, xflag = 0, qflag = 0;
int pflag_seen = 0;

/* Sometimes we want to print only succeeding syscalls. */
int not_failing_only = 0;

char *username = NULL;
uid_t run_uid;
gid_t run_gid;

int acolumn = DEFAULT_ACOLUMN;
int max_strlen = DEFAULT_STRLEN;
char *outfname = NULL;
FILE *outf;
struct tcb **tcbtab;
unsigned int nprocs, tcbtabsize;
char *progname;
extern const char version[];
extern char **environ;

static int trace P((void));
static void cleanup P((void));
static void interrupt P((int sig));
static sigset_t empty_set, blocked_set;

#ifdef HAVE_SIG_ATOMIC_T
static volatile sig_atomic_t interrupted;
#else /* !HAVE_SIG_ATOMIC_T */
#ifdef __STDC__
static volatile int interrupted;
#else /* !__STDC__ */
static int interrupted;
#endif /* !__STDC__ */
#endif /* !HAVE_SIG_ATOMIC_T */

#ifdef USE_PROCFS

static struct tcb *pfd2tcb P((int pfd));
static void reaper P((int sig));
static void rebuild_pollv P((void));
static struct pollfd *pollv;

#ifndef HAVE_POLLABLE_PROCFS

static void proc_poll_open P((void));
static void proc_poller P((int pfd));

struct proc_pollfd {
	int fd;
	int revents;
	int pid;
};

static int poller_pid;
static int proc_poll_pipe[2] = { -1, -1 };

#endif /* !HAVE_POLLABLE_PROCFS */

#ifdef HAVE_MP_PROCFS
#define POLLWANT	POLLWRNORM
#else
#define POLLWANT	POLLPRI
#endif
#endif /* USE_PROCFS */

static void
usage(ofp, exitval)
FILE *ofp;
int exitval;
{
	fprintf(ofp, "\
usage: strace [-dffhiqrtttTvVxx] [-a column] [-e expr] ... [-o file]\n\
              [-p pid] ... [-s strsize] [-u username] [command [arg ...]]\n\
   or: strace -c [-e expr] ... [-O overhead] [-S sortby] [command [arg ...]]\n\
-c -- count time, calls, and errors for each syscall and report summary\n\
-f -- follow forks, -ff -- with output into separate files\n\
-F -- attempt to follow vforks, -h -- print help message\n\
-i -- print instruction pointer at time of syscall\n\
-q -- suppress messages about attaching, detaching, etc.\n\
-r -- print relative timestamp, -t -- absolute timestamp, -tt -- with usecs\n\
-T -- print time spent in each syscall, -V -- print version\n\
-v -- verbose mode: print unabbreviated argv, stat, termio[s], etc. args\n\
-x -- print non-ascii strings in hex, -xx -- print all strings in hex\n\
-a column -- alignment COLUMN for printing syscall results (default %d)\n\
-e expr -- a qualifying expression: option=[!]all or option=[!]val1[,val2]...\n\
   options: trace, abbrev, verbose, raw, signal, read, or write\n\
-o file -- send trace output to FILE instead of stderr\n\
-O overhead -- set overhead for tracing syscalls to OVERHEAD usecs\n\
-p pid -- trace process with process id PID, may be repeated\n\
-s strsize -- limit length of print strings to STRSIZE chars (default %d)\n\
-S sortby -- sort syscall counts by: time, calls, name, nothing (default %s)\n\
-u username -- run command as username handling setuid and/or setgid\n\
-z -- print only succeeding syscalls\n\
", DEFAULT_ACOLUMN, DEFAULT_STRLEN, DEFAULT_SORTBY);
	exit(exitval);
}

#ifdef SVR4
#ifdef MIPS
void
foobar()
{
}
#endif /* MIPS */
#endif /* SVR4 */

int
main(argc, argv)
int argc;
char *argv[];
{
	extern int optind;
	extern char *optarg;
	struct tcb *tcp;
	int c, pid = 0;
	struct sigaction sa;

	static char buf[BUFSIZ];

	/* Allocate the initial tcbtab.  */
	tcbtabsize = argc;	/* Surely enough for all -p args.  */
	tcbtab = (struct tcb **) malloc (tcbtabsize * sizeof tcbtab[0]);
	tcbtab[0] = (struct tcb *) calloc (tcbtabsize, sizeof *tcbtab[0]);
	for (tcp = tcbtab[0]; tcp < &tcbtab[0][tcbtabsize]; ++tcp)
		tcbtab[tcp - tcbtab[0]] = &tcbtab[0][tcp - tcbtab[0]];

	progname = argv[0];
	outf = stderr;
	interactive = 1;
	qualify("trace=all");
	qualify("abbrev=all");
	qualify("verbose=all");
	qualify("signal=all");
	set_sortby(DEFAULT_SORTBY);
	set_personality(DEFAULT_PERSONALITY);
	while ((c = getopt(argc, argv,
		"+cdfFhiqrtTvVxza:e:o:O:p:s:S:u:")) != EOF) {
		switch (c) {
		case 'c':
			cflag++;
			dtime++;
			break;
		case 'd':
			debug++;
			break;
		case 'f':
			followfork++;
			break;
		case 'F':
			followvfork++;
			break;
		case 'h':
			usage(stdout, 0);
			break;
		case 'i':
			iflag++;
			break;
		case 'q':
			qflag++;
			break;
		case 'r':
			rflag++;
			tflag++;
			break;
		case 't':
			tflag++;
			break;
		case 'T':
			dtime++;
			break;
		case 'x':
			xflag++;
			break;
		case 'v':
			qualify("abbrev=none");
			break;
		case 'V':
			printf("%s\n", version);
			exit(0);
			break;
		case 'z':
			not_failing_only = 1;
			break;
		case 'a':
			acolumn = atoi(optarg);
			break;
		case 'e':
			qualify(optarg);
			break;
		case 'o':
			outfname = strdup(optarg);
			break;
		case 'O':
			set_overhead(atoi(optarg));
			break;
		case 'p':
			if ((pid = atoi(optarg)) == 0) {
				fprintf(stderr, "%s: Invalid process id: %s\n",
					progname, optarg);
				break;
			}
			if (pid == getpid()) {
				fprintf(stderr, "%s: I'm sorry, I can't let you do that, Dave.\n", progname);
				break;
			}
			if ((tcp = alloctcb(pid)) == NULL) {
				fprintf(stderr, "%s: tcb table full, please recompile strace\n",
					progname);
				exit(1);
			}
			tcp->flags |= TCB_ATTACHED;
			pflag_seen++;
			break;
		case 's':
			max_strlen = atoi(optarg);
			break;
		case 'S':
			set_sortby(optarg);
			break;
		case 'u':
			username = strdup(optarg);
			break;
		default:
			usage(stderr, 1);
			break;
		}
	}

	/* See if they want to run as another user. */
	if (username != NULL) {
		struct passwd *pent;

		if (getuid() != 0 || geteuid() != 0) {
			fprintf(stderr,
				"%s: you must be root to use the -u option\n",
				progname);
			exit(1);
		}
		if ((pent = getpwnam(username)) == NULL) {
			fprintf(stderr, "%s: cannot find user `%s'\n",
				progname, optarg);
			exit(1);
		}
		run_uid = pent->pw_uid;
		run_gid = pent->pw_gid;
	}
	else {
		run_uid = getuid();
		run_gid = getgid();
	}

#ifndef SVR4
	setreuid(geteuid(), getuid());
#endif

	/* See if they want to pipe the output. */
	if (outfname && (outfname[0] == '|' || outfname[0] == '!')) {
		if ((outf = popen(outfname + 1, "w")) == NULL) {
			fprintf(stderr, "%s: can't popen '%s': %s\n",
				progname, outfname + 1, strerror(errno));
			exit(1);
		}
		free(outfname);
		outfname = NULL;
	}

	/* Check if they want to redirect the output. */
	if (outfname) {
		long f;

		if ((outf = fopen(outfname, "w")) == NULL) {
			fprintf(stderr, "%s: can't fopen '%s': %s\n",
				progname, outfname, strerror(errno));
			exit(1);
		}

		if ((f=fcntl(fileno(outf), F_GETFD)) < 0 ) {
			perror("failed to get flags for outputfile");
			exit(1);
		}

		if (fcntl(fileno(outf), F_SETFD, f|FD_CLOEXEC) < 0 ) {
			perror("failed to set flags for outputfile");
			exit(1);
		}
	}

#ifndef SVR4
	setreuid(geteuid(), getuid());
#endif

	if (!outfname) {
		qflag = 1;
		setvbuf(outf, buf, _IOLBF, BUFSIZ);
	}
	else if (optind < argc)
		interactive = 0;
	else
		qflag = 1;

	for (c = 0; c < tcbtabsize; c++) {
		tcp = tcbtab[c];
		/* Reinitialize the output since it may have changed. */
		tcp->outf = outf;
		if (!(tcp->flags & TCB_INUSE) || !(tcp->flags & TCB_ATTACHED))
			continue;
#ifdef USE_PROCFS
		if (proc_open(tcp, 1) < 0) {
			fprintf(stderr, "trouble opening proc file\n");
			droptcb(tcp);
			continue;
		}
#else /* !USE_PROCFS */
		if (ptrace(PTRACE_ATTACH, tcp->pid, (char *) 1, 0) < 0) {
			perror("attach: ptrace(PTRACE_ATTACH, ...)");
			droptcb(tcp);
			continue;
		}
#endif /* !USE_PROCFS */
		if (!qflag)
			fprintf(stderr,
				"Process %u attached - interrupt to quit\n",
				pid);
	}

	if (optind < argc) {
		struct stat statbuf;
		char *filename;
		char pathname[MAXPATHLEN];

		filename = argv[optind];
		if (strchr(filename, '/'))
			strcpy(pathname, filename);
#ifdef USE_DEBUGGING_EXEC
		/*
		 * Debuggers customarily check the current directory
		 * first regardless of the path but doing that gives
		 * security geeks a panic attack.
		 */
		else if (stat(filename, &statbuf) == 0)
			strcpy(pathname, filename);
#endif /* USE_DEBUGGING_EXEC */
		else {
			char *path;
			int m, n, len;

			for (path = getenv("PATH"); path && *path; path += m) {
				if (strchr(path, ':')) {
					n = strchr(path, ':') - path;
					m = n + 1;
				}
				else
					m = n = strlen(path);
				if (n == 0) {
					getcwd(pathname, MAXPATHLEN);
					len = strlen(pathname);
				}
				else {
					strncpy(pathname, path, n);
					len = n;
				}
				if (len && pathname[len - 1] != '/')
					pathname[len++] = '/';
				strcpy(pathname + len, filename);
				if (stat(pathname, &statbuf) == 0)
					break;
			}
		}
		if (stat(pathname, &statbuf) < 0) {
			fprintf(stderr, "%s: %s: command not found\n",
				progname, filename);
			exit(1);
		}
		switch (pid = fork()) {
		case -1:
			perror("strace: fork");
			cleanup();
			exit(1);
			break;
		case 0: {
#ifdef USE_PROCFS
		        if (outf != stderr) close (fileno (outf));
#ifdef MIPS
			/* Kludge for SGI, see proc_open for details. */
			sa.sa_handler = foobar;
			sa.sa_flags = 0;
			sigemptyset(&sa.sa_mask);
			sigaction(SIGINT, &sa, NULL);
#endif /* MIPS */
#ifndef FREEBSD
			pause();
#else /* FREEBSD */
			kill(getpid(), SIGSTOP); /* stop HERE */
#endif /* FREEBSD */
#else /* !USE_PROCFS */
			if (outf!=stderr)
				close(fileno (outf));

			if (ptrace(PTRACE_TRACEME, 0, (char *) 1, 0) < 0) {
				perror("strace: ptrace(PTRACE_TRACEME, ...)");
				return -1;
			}
			if (debug)
				kill(getpid(), SIGSTOP);

			if (username != NULL || geteuid() == 0) {
				uid_t run_euid = run_uid;
				gid_t run_egid = run_gid;

				if (statbuf.st_mode & S_ISUID)
					run_euid = statbuf.st_uid;
				if (statbuf.st_mode & S_ISGID)
					run_egid = statbuf.st_gid;

				/*
				 * It is important to set groups before we
				 * lose privileges on setuid.
				 */
				if (username != NULL) {
					if (initgroups(username, run_gid) < 0) {
						perror("initgroups");
						exit(1);
					}
					if (setregid(run_gid, run_egid) < 0) {
						perror("setregid");
						exit(1);
					}
					if (setreuid(run_uid, run_euid) < 0) {
						perror("setreuid");
						exit(1);
					}
				}
			}
			else
				setreuid(run_uid, run_uid);
#endif /* !USE_PROCFS */

			execv(pathname, &argv[optind]);
			perror("strace: exec");
			_exit(1);
			break;
		}
		default:
			if ((tcp = alloctcb(pid)) == NULL) {
				fprintf(stderr, "tcb table full\n");
				cleanup();
				exit(1);
			}
#ifdef USE_PROCFS
			if (proc_open(tcp, 0) < 0) {
				fprintf(stderr, "trouble opening proc file\n");
				cleanup();
				exit(1);
			}
#endif /* USE_PROCFS */
#ifndef USE_PROCFS
			fake_execve(tcp, pathname, &argv[optind], environ);
#endif /* !USE_PROCFS */
			break;
		}
	}
	else if (pflag_seen == 0)
		usage(stderr, 1);

	sigemptyset(&empty_set);
	sigemptyset(&blocked_set);
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGTTOU, &sa, NULL);
	sigaction(SIGTTIN, &sa, NULL);
	if (interactive) {
		sigaddset(&blocked_set, SIGHUP);
		sigaddset(&blocked_set, SIGINT);
		sigaddset(&blocked_set, SIGQUIT);
		sigaddset(&blocked_set, SIGPIPE);
		sigaddset(&blocked_set, SIGTERM);
		sa.sa_handler = interrupt;
#ifdef SUNOS4
		/* POSIX signals on sunos4.1 are a little broken. */
		sa.sa_flags = SA_INTERRUPT;
#endif /* SUNOS4 */
	}
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
#ifdef USE_PROCFS
	sa.sa_handler = reaper;
	sigaction(SIGCHLD, &sa, NULL);
#else
	/* Make sure SIGCHLD has the default action so that waitpid
	   definitely works without losing track of children.  The user
	   should not have given us a bogus state to inherit, but he might
	   have.  Arguably we should detect SIG_IGN here and pass it on
	   to children, but probably noone really needs that.  */
	sa.sa_handler = SIG_DFL;
	sigaction(SIGCHLD, &sa, NULL);
#endif /* USE_PROCFS */

	if (trace() < 0)
		exit(1);
	cleanup();
	exit(0);
}

void
newoutf(tcp)
struct tcb *tcp;
{
	char name[MAXPATHLEN];
	FILE *fp;

	if (outfname && followfork > 1) {
		sprintf(name, "%s.%u", outfname, tcp->pid);
#ifndef SVR4
		setreuid(geteuid(), getuid());
#endif
		fp = fopen(name, "w");
#ifndef SVR4
		setreuid(geteuid(), getuid());
#endif
		if (fp == NULL) {
			perror("fopen");
			return;
		}
		tcp->outf = fp;
	}
	return;
}

struct tcb *
alloctcb(pid)
int pid;
{
	int i;
	struct tcb *tcp;

	for (i = 0; i < tcbtabsize; i++) {
		tcp = tcbtab[i];
		if ((tcp->flags & TCB_INUSE) == 0) {
			tcp->pid = pid;
			tcp->parent = NULL;
			tcp->nchildren = 0;
#ifdef TCB_CLONE_THREAD
			tcp->nclone_threads = tcp->nclone_detached = 0;
			tcp->nclone_waiting = 0;
#endif
			tcp->flags = TCB_INUSE | TCB_STARTUP;
			tcp->outf = outf; /* Initialise to current out file */
			tcp->stime.tv_sec = 0;
			tcp->stime.tv_usec = 0;
			tcp->pfd = -1;
			nprocs++;
			return tcp;
		}
	}
	return NULL;
}

#ifdef USE_PROCFS
int
proc_open(tcp, attaching)
struct tcb *tcp;
int attaching;
{
	char proc[32];
	long arg;
#ifdef SVR4
	int i;
	sysset_t syscalls;
	sigset_t signals;
	fltset_t faults;
#endif
#ifndef HAVE_POLLABLE_PROCFS
	static int last_pfd;
#endif

#ifdef HAVE_MP_PROCFS
	/* Open the process pseudo-files in /proc. */
	sprintf(proc, "/proc/%d/ctl", tcp->pid);
	if ((tcp->pfd = open(proc, O_WRONLY|O_EXCL)) < 0) {
		perror("strace: open(\"/proc/...\", ...)");
		return -1;
	}
	if ((arg = fcntl(tcp->pfd, F_GETFD)) < 0) {
		perror("F_GETFD");
		return -1;
	}
	if (fcntl(tcp->pfd, F_SETFD, arg|FD_CLOEXEC) < 0) {
		perror("F_SETFD");
		return -1;
	}
	sprintf(proc, "/proc/%d/status", tcp->pid);
	if ((tcp->pfd_stat = open(proc, O_RDONLY|O_EXCL)) < 0) {
		perror("strace: open(\"/proc/...\", ...)");
		return -1;
	}
	if ((arg = fcntl(tcp->pfd_stat, F_GETFD)) < 0) {
		perror("F_GETFD");
		return -1;
	}
	if (fcntl(tcp->pfd_stat, F_SETFD, arg|FD_CLOEXEC) < 0) {
		perror("F_SETFD");
		return -1;
	}
	sprintf(proc, "/proc/%d/as", tcp->pid);
	if ((tcp->pfd_as = open(proc, O_RDONLY|O_EXCL)) < 0) {
		perror("strace: open(\"/proc/...\", ...)");
		return -1;
	}
	if ((arg = fcntl(tcp->pfd_as, F_GETFD)) < 0) {
		perror("F_GETFD");
		return -1;
	}
	if (fcntl(tcp->pfd_as, F_SETFD, arg|FD_CLOEXEC) < 0) {
		perror("F_SETFD");
		return -1;
	}
#else
	/* Open the process pseudo-file in /proc. */
#ifndef FREEBSD
	sprintf(proc, "/proc/%d", tcp->pid);
	if ((tcp->pfd = open(proc, O_RDWR|O_EXCL)) < 0) {
#else /* FREEBSD */
	sprintf(proc, "/proc/%d/mem", tcp->pid);
	if ((tcp->pfd = open(proc, O_RDWR)) < 0) {
#endif /* FREEBSD */
		perror("strace: open(\"/proc/...\", ...)");
		return -1;
	}
	if ((arg = fcntl(tcp->pfd, F_GETFD)) < 0) {
		perror("F_GETFD");
		return -1;
	}
	if (fcntl(tcp->pfd, F_SETFD, arg|FD_CLOEXEC) < 0) {
		perror("F_SETFD");
		return -1;
	}
#endif
#ifdef FREEBSD
	sprintf(proc, "/proc/%d/regs", tcp->pid);
	if ((tcp->pfd_reg = open(proc, O_RDONLY)) < 0) {
		perror("strace: open(\"/proc/.../regs\", ...)");
		return -1;
	}
	if (cflag) {
		sprintf(proc, "/proc/%d/status", tcp->pid);
		if ((tcp->pfd_status = open(proc, O_RDONLY)) < 0) {
			perror("strace: open(\"/proc/.../status\", ...)");
			return -1;
		}
	} else
		tcp->pfd_status = -1;
#endif /* FREEBSD */
	rebuild_pollv();
	if (!attaching) {
		/*
		 * Wait for the child to pause.  Because of a race
		 * condition we have to poll for the event.
		 */
		for (;;) {
			if (IOCTL_STATUS (tcp) < 0) {
				perror("strace: PIOCSTATUS");
				return -1;
			}
			if (tcp->status.PR_FLAGS & PR_ASLEEP)
			    break;
		}
	}
#ifndef FREEBSD
	/* Stop the process so that we own the stop. */
	if (IOCTL(tcp->pfd, PIOCSTOP, (char *)NULL) < 0) {
		perror("strace: PIOCSTOP");
		return -1;
	}
#endif
#ifdef PIOCSET
	/* Set Run-on-Last-Close. */
	arg = PR_RLC;
	if (IOCTL(tcp->pfd, PIOCSET, &arg) < 0) {
		perror("PIOCSET PR_RLC");
		return -1;
	}
	/* Set or Reset Inherit-on-Fork. */
	arg = PR_FORK;
	if (IOCTL(tcp->pfd, followfork ? PIOCSET : PIOCRESET, &arg) < 0) {
		perror("PIOC{SET,RESET} PR_FORK");
		return -1;
	}
#else  /* !PIOCSET */
#ifndef FREEBSD
	if (ioctl(tcp->pfd, PIOCSRLC) < 0) {
		perror("PIOCSRLC");
		return -1;
	}
	if (ioctl(tcp->pfd, followfork ? PIOCSFORK : PIOCRFORK) < 0) {
		perror("PIOC{S,R}FORK");
		return -1;
	}
#else /* FREEBSD */
	/* just unset the PF_LINGER flag for the Run-on-Last-Close. */
	if (ioctl(tcp->pfd, PIOCGFL, &arg) < 0) {
	        perror("PIOCGFL");
	        return -1;
	}
	arg &= ~PF_LINGER;
	if (ioctl(tcp->pfd, PIOCSFL, arg) < 0) {
	        perror("PIOCSFL");
	        return -1;
	}
#endif /* FREEBSD */
#endif /* !PIOCSET */
#ifndef FREEBSD
	/* Enable all syscall entries we care about. */
	premptyset(&syscalls);
	for (i = 1; i < MAX_QUALS; ++i) {
		if (i > (sizeof syscalls) * CHAR_BIT) break;
		if (qual_flags [i] & QUAL_TRACE) praddset (&syscalls, i);
	}
	praddset (&syscalls, SYS_execve);
	if (followfork) {
		praddset (&syscalls, SYS_fork);
#ifdef SYS_forkall
		praddset (&syscalls, SYS_forkall);
#endif
#ifdef SYS_fork1
		praddset (&syscalls, SYS_fork1);
#endif
#ifdef SYS_rfork1
		praddset (&syscalls, SYS_rfork1);
#endif
#ifdef SYS_rforkall
		praddset (&syscalls, SYS_rforkall);
#endif
	}
	if (IOCTL(tcp->pfd, PIOCSENTRY, &syscalls) < 0) {
		perror("PIOCSENTRY");
		return -1;
	}
	/* Enable the syscall exits. */
	if (IOCTL(tcp->pfd, PIOCSEXIT, &syscalls) < 0) {
		perror("PIOSEXIT");
		return -1;
	}
	/* Enable signals we care about. */
	premptyset(&signals);
	for (i = 1; i < MAX_QUALS; ++i) {
		if (i > (sizeof signals) * CHAR_BIT) break;
		if (qual_flags [i] & QUAL_SIGNAL) praddset (&signals, i);
	}
	if (IOCTL(tcp->pfd, PIOCSTRACE, &signals) < 0) {
		perror("PIOCSTRACE");
		return -1;
	}
	/* Enable faults we care about */
	premptyset(&faults);
	for (i = 1; i < MAX_QUALS; ++i) {
		if (i > (sizeof faults) * CHAR_BIT) break;
		if (qual_flags [i] & QUAL_FAULT) praddset (&faults, i);
	}
	if (IOCTL(tcp->pfd, PIOCSFAULT, &faults) < 0) {
		perror("PIOCSFAULT");
		return -1;
	}
#else /* FREEBSD */
	/* set events flags. */
	arg = S_SIG | S_SCE | S_SCX ;
	if(ioctl(tcp->pfd, PIOCBIS, arg) < 0) {
		perror("PIOCBIS");
		return -1;
	}
#endif /* FREEBSD */
	if (!attaching) {
#ifdef MIPS
		/*
		 * The SGI PRSABORT doesn't work for pause() so
		 * we send it a caught signal to wake it up.
		 */
		kill(tcp->pid, SIGINT);
#else /* !MIPS */
#ifdef PRSABORT
		/* The child is in a pause(), abort it. */
		arg = PRSABORT;
		if (IOCTL (tcp->pfd, PIOCRUN, &arg) < 0) {
			perror("PIOCRUN");
			return -1;
		}
#endif
#endif /* !MIPS*/
#ifdef FREEBSD
		/* wake up the child if it received the SIGSTOP */
		kill(tcp->pid, SIGCONT);
#endif
		for (;;) {
			/* Wait for the child to do something. */
			if (IOCTL_WSTOP (tcp) < 0) {
				perror("PIOCWSTOP");
				return -1;
			}
			if (tcp->status.PR_WHY == PR_SYSENTRY) {
				tcp->flags &= ~TCB_INSYSCALL;
				get_scno(tcp);
				if (tcp->scno == SYS_execve)
					break;
			}
			/* Set it running: maybe execve will be next. */
#ifndef FREEBSD
			arg = 0;
			if (IOCTL(tcp->pfd, PIOCRUN, &arg) < 0) {
#else /* FREEBSD */
			if (IOCTL(tcp->pfd, PIOCRUN, 0) < 0) {
#endif /* FREEBSD */
				perror("PIOCRUN");
				return -1;
			}
#ifdef FREEBSD
			/* handle the case where we "opened" the child before
			   it did the kill -STOP */
			if (tcp->status.PR_WHY == PR_SIGNALLED &&
			    tcp->status.PR_WHAT == SIGSTOP)
			        kill(tcp->pid, SIGCONT);
#endif
		}
#ifndef FREEBSD
	}
#else /* FREEBSD */
	} else {
		if (attaching < 2) {
			/* We are attaching to an already running process.
			 * Try to figure out the state of the process in syscalls,
			 * to handle the first event well.
			 * This is done by having a look at the "wchan" property of the
			 * process, which tells where it is stopped (if it is). */
			FILE * status;
			char wchan[20]; /* should be enough */

			sprintf(proc, "/proc/%d/status", tcp->pid);
			status = fopen(proc, "r");
			if (status &&
			    (fscanf(status, "%*s %*d %*d %*d %*d %*d,%*d %*s %*d,%*d"
				    "%*d,%*d %*d,%*d %19s", wchan) == 1) &&
			    strcmp(wchan, "nochan") && strcmp(wchan, "spread") &&
			    strcmp(wchan, "stopevent")) {
				/* The process is asleep in the middle of a syscall.
				   Fake the syscall entry event */
				tcp->flags &= ~(TCB_INSYSCALL|TCB_STARTUP);
				tcp->status.PR_WHY = PR_SYSENTRY;
				trace_syscall(tcp);
			}
			if (status)
				fclose(status);
		} /* otherwise it's a fork being followed */
	}
#endif /* FREEBSD */
#ifndef HAVE_POLLABLE_PROCFS
	if (proc_poll_pipe[0] != -1)
		proc_poller(tcp->pfd);
	else if (nprocs > 1) {
		proc_poll_open();
		proc_poller(last_pfd);
		proc_poller(tcp->pfd);
	}
	last_pfd = tcp->pfd;
#endif /* !HAVE_POLLABLE_PROCFS */
	return 0;
}

#endif /* USE_PROCFS */

struct tcb *
pid2tcb(pid)
int pid;
{
	int i;
	struct tcb *tcp;

	for (i = 0; i < tcbtabsize; i++) {
		tcp = tcbtab[i];
		if (pid && tcp->pid != pid)
			continue;
		if (tcp->flags & TCB_INUSE)
			return tcp;
	}
	return NULL;
}

#ifdef USE_PROCFS

static struct tcb *
pfd2tcb(pfd)
int pfd;
{
	int i;

	for (i = 0; i < tcbtabsize; i++) {
		struct tcb *tcp = tcbtab[i];
		if (tcp->pfd != pfd)
			continue;
		if (tcp->flags & TCB_INUSE)
			return tcp;
	}
	return NULL;
}

#endif /* USE_PROCFS */

void
droptcb(tcp)
struct tcb *tcp;
{
	if (tcp->pid == 0)
		return;
#ifdef TCB_CLONE_THREAD
	if (tcp->nclone_threads > 0) {
		/* There are other threads left in this process, but this
		   is the one whose PID represents the whole process.
		   We need to keep this record around as a zombie until
		   all the threads die.  */
		tcp->flags |= TCB_EXITING;
		return;
	}
#endif
	nprocs--;
	tcp->pid = 0;

	if (tcp->parent != NULL) {
		tcp->parent->nchildren--;
#ifdef TCB_CLONE_THREAD
		if (tcp->flags & TCB_CLONE_DETACHED)
			tcp->parent->nclone_detached--;
		if (tcp->flags & TCB_CLONE_THREAD)
			tcp->parent->nclone_threads--;
#endif
		tcp->parent = NULL;
	}

	tcp->flags = 0;
	if (tcp->pfd != -1) {
		close(tcp->pfd);
		tcp->pfd = -1;
#ifdef FREEBSD
		if (tcp->pfd_reg != -1) {
		        close(tcp->pfd_reg);
		        tcp->pfd_reg = -1;
		}
		if (tcp->pfd_status != -1) {
			close(tcp->pfd_status);
			tcp->pfd_status = -1;
		}
#endif /* !FREEBSD */
#ifdef USE_PROCFS
		rebuild_pollv(); /* Note, flags needs to be cleared by now.  */
#endif
	}

	if (outfname && followfork > 1 && tcp->outf)
		fclose(tcp->outf);

	tcp->outf = 0;
}

#ifndef USE_PROCFS

static int
resume(tcp)
struct tcb *tcp;
{
	if (tcp == NULL)
		return -1;

	if (!(tcp->flags & TCB_SUSPENDED)) {
		fprintf(stderr, "PANIC: pid %u not suspended\n", tcp->pid);
		return -1;
	}
	tcp->flags &= ~TCB_SUSPENDED;
#ifdef TCB_CLONE_THREAD
	if (tcp->flags & TCB_CLONE_THREAD)
		tcp->parent->nclone_waiting--;
#endif

	if (ptrace(PTRACE_SYSCALL, tcp->pid, (char *) 1, 0) < 0) {
		perror("resume: ptrace(PTRACE_SYSCALL, ...)");
		return -1;
	}

	if (!qflag)
		fprintf(stderr, "Process %u resumed\n", tcp->pid);
	return 0;
}

#endif /* !USE_PROCFS */

/* detach traced process; continue with sig */

static int
detach(tcp, sig)
struct tcb *tcp;
int sig;
{
	int error = 0;
#ifdef LINUX
	int status, resumed;
#endif

	if (tcp->flags & TCB_BPTSET)
		sig = SIGKILL;

#ifdef LINUX
	/*
	 * Linux wrongly insists the child be stopped
	 * before detaching.  Arghh.  We go through hoops
	 * to make a clean break of things.
	 */
#if defined(SPARC)
#undef PTRACE_DETACH
#define PTRACE_DETACH PTRACE_SUNDETACH
#endif
	if ((error = ptrace(PTRACE_DETACH, tcp->pid, (char *) 1, sig)) == 0) {
		/* On a clear day, you can see forever. */
	}
	else if (errno != ESRCH) {
		/* Shouldn't happen. */
		perror("detach: ptrace(PTRACE_DETACH, ...)");
	}
	else if (kill(tcp->pid, 0) < 0) {
		if (errno != ESRCH)
			perror("detach: checking sanity");
	}
	else if (kill(tcp->pid, SIGSTOP) < 0) {
		if (errno != ESRCH)
			perror("detach: stopping child");
	}
	else {
		for (;;) {
#ifdef __WALL
			if (wait4(tcp->pid, &status, __WALL, NULL) < 0) {
				if (errno == ECHILD) /* Already gone.  */
					break;
				if (errno != EINVAL) {
					perror("detach: waiting");
					break;
				}
#endif /* __WALL */
				/* No __WALL here.  */
				if (waitpid(tcp->pid, &status, 0) < 0) {
					if (errno != ECHILD) {
						perror("detach: waiting");
						break;
					}
#ifdef __WCLONE
					/* If no processes, try clones.  */
					if (wait4(tcp->pid, &status, __WCLONE,
						  NULL) < 0) {
						if (errno != ECHILD)
							perror("detach: waiting");
						break;
					}
#endif /* __WCLONE */
				}
#ifdef __WALL
			}
#endif
			if (!WIFSTOPPED(status)) {
				/* Au revoir, mon ami. */
				break;
			}
			if (WSTOPSIG(status) == SIGSTOP) {
				if ((error = ptrace(PTRACE_DETACH,
				    tcp->pid, (char *) 1, sig)) < 0) {
					if (errno != ESRCH)
						perror("detach: ptrace(PTRACE_DETACH, ...)");
					/* I died trying. */
				}
				break;
			}
			if ((error = ptrace(PTRACE_CONT, tcp->pid, (char *) 1,
			    WSTOPSIG(status) == SIGTRAP ?
			    0 : WSTOPSIG(status))) < 0) {
				if (errno != ESRCH)
					perror("detach: ptrace(PTRACE_CONT, ...)");
				break;
			}
		}
	}
#endif /* LINUX */

#if defined(SUNOS4)
	/* PTRACE_DETACH won't respect `sig' argument, so we post it here. */
	if (sig && kill(tcp->pid, sig) < 0)
		perror("detach: kill");
	sig = 0;
	if ((error = ptrace(PTRACE_DETACH, tcp->pid, (char *) 1, sig)) < 0)
		perror("detach: ptrace(PTRACE_DETACH, ...)");
#endif /* SUNOS4 */

#ifndef USE_PROCFS
	resumed = 0;

	/* XXX This won't always be quite right (but it never was).
	   A waiter with argument 0 or < -1 is waiting for any pid in
	   a particular pgrp, which this child might or might not be
	   in.  The waiter will only wake up if it's argument is -1
	   or if it's waiting for tcp->pid's pgrp.  It makes a
	   difference to wake up a waiter when there might be more
	   traced children, because it could get a false ECHILD
	   error.  OTOH, if this was the last child in the pgrp, then
	   it ought to wake up and get ECHILD.  We would have to
	   search the system for all pid's in the pgrp to be sure.

	     && (t->waitpid == -1 ||
		 (t->waitpid == 0 && getpgid (tcp->pid) == getpgid (t->pid))
		 || (t->waitpid < 0 && t->waitpid == -getpid (t->pid)))
	*/

	if (tcp->parent &&
	    (tcp->parent->flags & TCB_SUSPENDED) &&
	    (tcp->parent->waitpid <= 0 || tcp->parent->waitpid == tcp->pid)) {
 		error = resume(tcp->parent);
		++resumed;
	}
#ifdef TCB_CLONE_THREAD
	if (tcp->parent && tcp->parent->nclone_waiting > 0) {
		/* Some other threads of our parent are waiting too.  */
		unsigned int i;

		/* Resume all the threads that were waiting for this PID.  */
		for (i = 0; i < tcbtabsize; i++) {
			struct tcb *t = tcbtab[i];
			if (t->parent == tcp->parent && t != tcp
			    && ((t->flags & (TCB_CLONE_THREAD|TCB_SUSPENDED))
				== (TCB_CLONE_THREAD|TCB_SUSPENDED))
			    && t->waitpid == tcp->pid) {
				error |= resume (t);
				++resumed;
			}
		}
		if (resumed == 0)
			/* Noone was waiting for this PID in particular,
			   so now we might need to resume some wildcarders.  */
			for (i = 0; i < tcbtabsize; i++) {
				struct tcb *t = tcbtab[i];
				if (t->parent == tcp->parent && t != tcp
				    && ((t->flags
					 & (TCB_CLONE_THREAD|TCB_SUSPENDED))
					== (TCB_CLONE_THREAD|TCB_SUSPENDED))
				    && t->waitpid <= 0
					) {
					error |= resume (t);
					break;
				}
			}
	}
#endif

#endif /* !USE_PROCFS */

	if (!qflag)
		fprintf(stderr, "Process %u detached\n", tcp->pid);

	droptcb(tcp);
	return error;
}

#ifdef USE_PROCFS

static void
reaper(sig)
int sig;
{
	int pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
#if 0
		struct tcb *tcp;

		tcp = pid2tcb(pid);
		if (tcp)
			droptcb(tcp);
#endif
	}
}

#endif /* USE_PROCFS */

static void
cleanup()
{
	int i;
	struct tcb *tcp;

	for (i = 0; i < tcbtabsize; i++) {
		tcp = tcbtab[i];
		if (!(tcp->flags & TCB_INUSE))
			continue;
		if (debug)
			fprintf(stderr,
				"cleanup: looking at pid %u\n", tcp->pid);
		if (tcp_last &&
		    (!outfname || followfork < 2 || tcp_last == tcp)) {
			tprintf(" <unfinished ...>\n");
			tcp_last = NULL;
		}
		if (tcp->flags & TCB_ATTACHED)
			detach(tcp, 0);
		else {
			kill(tcp->pid, SIGCONT);
			kill(tcp->pid, SIGTERM);
		}
	}
	if (cflag)
		call_summary(outf);
}

static void
interrupt(sig)
int sig;
{
	interrupted = 1;
}

#ifndef HAVE_STRERROR

#if !HAVE_DECL_SYS_ERRLIST
extern int sys_nerr;
extern char *sys_errlist[];
#endif /* HAVE_DECL_SYS_ERRLIST */

const char *
strerror(errno)
int errno;
{
	static char buf[64];

	if (errno < 1 || errno >= sys_nerr) {
		sprintf(buf, "Unknown error %d", errno);
		return buf;
	}
	return sys_errlist[errno];
}

#endif /* HAVE_STERRROR */

#ifndef HAVE_STRSIGNAL

#ifdef HAVE__SYS_SIGLIST
#if !HAVE_DECL_SYS_SIGLIST
extern char *sys_siglist[];
	extern char *_sys_siglist[];
#else
#endif
#endif /* SYS_SIGLIST_DECLARED */

const char *
strsignal(sig)
int sig;
{
	static char buf[64];

	if (sig < 1 || sig >= NSIG) {
		sprintf(buf, "Unknown signal %d", sig);
		return buf;
	}
#ifdef HAVE__SYS_SIGLIST
	return _sys_siglist[sig];
#else
	return sys_siglist[sig];
#endif
}

#endif /* HAVE_STRSIGNAL */

#ifdef USE_PROCFS

static void
rebuild_pollv()
{
	int i, j;

	if (pollv != NULL)
		free (pollv);
	pollv = (struct pollfd *) malloc(nprocs * sizeof pollv[0]);
	if (pollv == NULL) {
		fprintf(stderr, "strace: out of memory for poll vector\n");
		exit(1);
	}

	for (i = j = 0; i < tcbtabsize; i++) {
		struct tcb *tcp = tcbtab[i];
		if (!(tcp->flags & TCB_INUSE))
			continue;
		pollv[j].fd = tcp->pfd;
		pollv[j].events = POLLWANT;
		j++;
	}
	if (j != nprocs) {
		fprintf(stderr, "strace: proc miscount\n");
		exit(1);
	}
}

#ifndef HAVE_POLLABLE_PROCFS

static void
proc_poll_open()
{
	int arg;
	int i;

	if (pipe(proc_poll_pipe) < 0) {
		perror("pipe");
		exit(1);
	}
	for (i = 0; i < 2; i++) {
		if ((arg = fcntl(proc_poll_pipe[i], F_GETFD)) < 0) {
			perror("F_GETFD");
			exit(1);
		}
		if (fcntl(proc_poll_pipe[i], F_SETFD, arg|FD_CLOEXEC) < 0) {
			perror("F_SETFD");
			exit(1);
		}
	}
}

static int
proc_poll(pollv, nfds, timeout)
struct pollfd *pollv;
int nfds;
int timeout;
{
	int i;
	int n;
	struct proc_pollfd pollinfo;

	if ((n = read(proc_poll_pipe[0], &pollinfo, sizeof(pollinfo))) < 0)
		return n;
	if (n != sizeof(struct proc_pollfd)) {
		fprintf(stderr, "panic: short read: %d\n", n);
		exit(1);
	}
	for (i = 0; i < nprocs; i++) {
		if (pollv[i].fd == pollinfo.fd)
			pollv[i].revents = pollinfo.revents;
		else
			pollv[i].revents = 0;
	}
	poller_pid = pollinfo.pid;
	return 1;
}

static void
wakeup_handler(sig)
int sig;
{
}

static void
proc_poller(pfd)
int pfd;
{
	struct proc_pollfd pollinfo;
	struct sigaction sa;
	sigset_t blocked_set, empty_set;
	int i;
	int n;
	struct rlimit rl;
#ifdef FREEBSD
	struct procfs_status pfs;
#endif /* FREEBSD */

	switch (fork()) {
	case -1:
		perror("fork");
		_exit(0);
	case 0:
		break;
	default:
		return;
	}

	sa.sa_handler = interactive ? SIG_DFL : SIG_IGN;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sa.sa_handler = wakeup_handler;
	sigaction(SIGUSR1, &sa, NULL);
	sigemptyset(&blocked_set);
	sigaddset(&blocked_set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &blocked_set, NULL);
	sigemptyset(&empty_set);

	if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
		perror("getrlimit(RLIMIT_NOFILE, ...)");
		_exit(0);
	}
	n = rl.rlim_cur;
	for (i = 0; i < n; i++) {
		if (i != pfd && i != proc_poll_pipe[1])
			close(i);
	}

	pollinfo.fd = pfd;
	pollinfo.pid = getpid();
	for (;;) {
#ifndef FREEBSD
	        if (ioctl(pfd, PIOCWSTOP, NULL) < 0)
#else /* FREEBSD */
	        if (ioctl(pfd, PIOCWSTOP, &pfs) < 0)
#endif /* FREEBSD */
		{
			switch (errno) {
			case EINTR:
				continue;
			case EBADF:
				pollinfo.revents = POLLERR;
				break;
			case ENOENT:
				pollinfo.revents = POLLHUP;
				break;
			default:
				perror("proc_poller: PIOCWSTOP");
			}
			write(proc_poll_pipe[1], &pollinfo, sizeof(pollinfo));
			_exit(0);
		}
		pollinfo.revents = POLLWANT;
		write(proc_poll_pipe[1], &pollinfo, sizeof(pollinfo));
		sigsuspend(&empty_set);
	}
}

#endif /* !HAVE_POLLABLE_PROCFS */

static int
choose_pfd()
{
	int i, j;
	struct tcb *tcp;

	static int last;

	if (followfork < 2 &&
	    last < nprocs && (pollv[last].revents & POLLWANT)) {
		/*
		 * The previous process is ready to run again.  We'll
		 * let it do so if it is currently in a syscall.  This
		 * heuristic improves the readability of the trace.
		 */
		tcp = pfd2tcb(pollv[last].fd);
		if (tcp && (tcp->flags & TCB_INSYSCALL))
			return pollv[last].fd;
	}

	for (i = 0; i < nprocs; i++) {
		/* Let competing children run round robin. */
		j = (i + last + 1) % nprocs;
		if (pollv[j].revents & (POLLHUP | POLLERR)) {
			tcp = pfd2tcb(pollv[j].fd);
			if (!tcp) {
				fprintf(stderr, "strace: lost proc\n");
				exit(1);
			}
			droptcb(tcp);
			return -1;
		}
		if (pollv[j].revents & POLLWANT) {
			last = j;
			return pollv[j].fd;
		}
	}
	fprintf(stderr, "strace: nothing ready\n");
	exit(1);
}

static int
trace()
{
#ifdef POLL_HACK
	struct tcb *in_syscall = NULL;
#endif
	struct tcb *tcp;
	int pfd;
	int what;
	int ioctl_result = 0, ioctl_errno = 0;
	long arg;

	for (;;) {
		if (interactive)
			sigprocmask(SIG_SETMASK, &empty_set, NULL);

		if (nprocs == 0)
			break;

		switch (nprocs) {
		case 1:
#ifndef HAVE_POLLABLE_PROCFS
			if (proc_poll_pipe[0] == -1) {
#endif
				tcp = pid2tcb(0);
				if (!tcp)
					continue;
				pfd = tcp->pfd;
				if (pfd == -1)
					continue;
				break;
#ifndef HAVE_POLLABLE_PROCFS
			}
			/* fall through ... */
#endif /* !HAVE_POLLABLE_PROCFS */
		default:
#ifdef HAVE_POLLABLE_PROCFS
#ifdef POLL_HACK
		        /* On some systems (e.g. UnixWare) we get too much ugly
			   "unfinished..." stuff when multiple proceses are in
			   syscalls.  Here's a nasty hack */

			if (in_syscall) {
				struct pollfd pv;
				tcp = in_syscall;
				in_syscall = NULL;
				pv.fd = tcp->pfd;
				pv.events = POLLWANT;
				if ((what = poll (&pv, 1, 1)) < 0) {
					if (interrupted)
						return 0;
					continue;
				}
				else if (what == 1 && pv.revents & POLLWANT) {
					goto FOUND;
				}
			}
#endif

			if (poll(pollv, nprocs, INFTIM) < 0) {
				if (interrupted)
					return 0;
				continue;
			}
#else /* !HAVE_POLLABLE_PROCFS */
			if (proc_poll(pollv, nprocs, INFTIM) < 0) {
				if (interrupted)
					return 0;
				continue;
			}
#endif /* !HAVE_POLLABLE_PROCFS */
			pfd = choose_pfd();
			if (pfd == -1)
				continue;
			break;
		}

		/* Look up `pfd' in our table. */
		if ((tcp = pfd2tcb(pfd)) == NULL) {
			fprintf(stderr, "unknown pfd: %u\n", pfd);
			exit(1);
		}
#ifdef POLL_HACK
	FOUND:
#endif
		/* Get the status of the process. */
		if (!interrupted) {
#ifndef FREEBSD
			ioctl_result = IOCTL_WSTOP (tcp);
#else /* FREEBSD */
			/* Thanks to some scheduling mystery, the first poller
			   sometimes waits for the already processed end of fork
			   event. Doing a non blocking poll here solves the problem. */
			if (proc_poll_pipe[0] != -1)
				ioctl_result = IOCTL_STATUS (tcp);
			else
			  	ioctl_result = IOCTL_WSTOP (tcp);
#endif /* FREEBSD */
			ioctl_errno = errno;
#ifndef HAVE_POLLABLE_PROCFS
			if (proc_poll_pipe[0] != -1) {
				if (ioctl_result < 0)
					kill(poller_pid, SIGKILL);
				else
					kill(poller_pid, SIGUSR1);
			}
#endif /* !HAVE_POLLABLE_PROCFS */
		}
		if (interrupted)
			return 0;

		if (interactive)
			sigprocmask(SIG_BLOCK, &blocked_set, NULL);

		if (ioctl_result < 0) {
			/* Find out what happened if it failed. */
			switch (ioctl_errno) {
			case EINTR:
			case EBADF:
				continue;
#ifdef FREEBSD
			case ENOTTY:
#endif
			case ENOENT:
				droptcb(tcp);
				continue;
			default:
				perror("PIOCWSTOP");
				exit(1);
			}
		}

#ifdef FREEBSD
		if ((tcp->flags & TCB_STARTUP) && (tcp->status.PR_WHY == PR_SYSEXIT)) {
			/* discard first event for a syscall we never entered */
			IOCTL (tcp->pfd, PIOCRUN, 0);
			continue;
		}
#endif

		/* clear the just started flag */
		tcp->flags &= ~TCB_STARTUP;

		/* set current output file */
		outf = tcp->outf;

		if (cflag) {
			struct timeval stime;
#ifdef FREEBSD
			char buf[1024];
			int len;

			if ((len = pread(tcp->pfd_status, buf, sizeof(buf) - 1, 0)) > 0) {
				buf[len] = '\0';
				sscanf(buf,
				       "%*s %*d %*d %*d %*d %*d,%*d %*s %*d,%*d %*d,%*d %ld,%ld",
				       &stime.tv_sec, &stime.tv_usec);
			} else
				stime.tv_sec = stime.tv_usec = 0;
#else /* !FREEBSD */
			stime.tv_sec = tcp->status.pr_stime.tv_sec;
			stime.tv_usec = tcp->status.pr_stime.tv_nsec/1000;
#endif /* !FREEBSD */
			tv_sub(&tcp->dtime, &stime, &tcp->stime);
			tcp->stime = stime;
		}
		what = tcp->status.PR_WHAT;
		switch (tcp->status.PR_WHY) {
#ifndef FREEBSD
		case PR_REQUESTED:
			if (tcp->status.PR_FLAGS & PR_ASLEEP) {
				tcp->status.PR_WHY = PR_SYSENTRY;
				if (trace_syscall(tcp) < 0) {
					fprintf(stderr, "syscall trouble\n");
					exit(1);
				}
			}
			break;
#endif /* !FREEBSD */
		case PR_SYSENTRY:
#ifdef POLL_HACK
		        in_syscall = tcp;
#endif
		case PR_SYSEXIT:
			if (trace_syscall(tcp) < 0) {
				fprintf(stderr, "syscall trouble\n");
				exit(1);
			}
			break;
		case PR_SIGNALLED:
			if (!cflag && (qual_flags[what] & QUAL_SIGNAL)) {
				printleader(tcp);
				tprintf("--- %s (%s) ---",
					signame(what), strsignal(what));
				printtrailer(tcp);
#ifdef PR_INFO
				if (tcp->status.PR_INFO.si_signo == what) {
					printleader(tcp);
					tprintf("    siginfo=");
					printsiginfo(&tcp->status.PR_INFO, 1);
					printtrailer(tcp);
				}
#endif
			}
			break;
		case PR_FAULTED:
			if (!cflag && (qual_flags[what] & QUAL_FAULT)) {
				printleader(tcp);
				tprintf("=== FAULT %d ===", what);
				printtrailer(tcp);
			}
			break;
#ifdef FREEBSD
		case 0: /* handle case we polled for nothing */
		  	continue;
#endif
		default:
			fprintf(stderr, "odd stop %d\n", tcp->status.PR_WHY);
			exit(1);
			break;
		}
		arg = 0;
#ifndef FREEBSD
		if (IOCTL (tcp->pfd, PIOCRUN, &arg) < 0) {
#else
		if (IOCTL (tcp->pfd, PIOCRUN, 0) < 0) {
#endif
			perror("PIOCRUN");
			exit(1);
		}
	}
	return 0;
}

#else /* !USE_PROCFS */

#ifdef TCB_GROUP_EXITING
/* Handle an exit detach or death signal that is taking all the
   related clone threads with it.  This is called in three circumstances:
   SIG == -1	TCP has already died (TCB_ATTACHED is clear, strace is parent).
   SIG == 0	Continuing TCP will perform an exit_group syscall.
   SIG == other	Continuing TCP with SIG will kill the process.
*/
static int
handle_group_exit(struct tcb *tcp, int sig)
{
	/* We need to locate our records of all the clone threads
	   related to TCP, either its children or siblings.  */
	struct tcb *leader = ((tcp->flags & TCB_CLONE_THREAD)
			      ? tcp->parent
			      : tcp->nclone_detached > 0
			      ? tcp : NULL);

	if (sig < 0) {
		if (leader != NULL && leader != tcp)
			fprintf(stderr,
				"PANIC: handle_group_exit: %d leader %d\n",
				tcp->pid, leader ? leader->pid : -1);
		droptcb(tcp);	/* Already died.  */
	}
	else {
		if (tcp->flags & TCB_ATTACHED) {
			if (leader != NULL && leader != tcp) {
				/* We need to detach the leader so that the
				   process death will be reported to its real
				   parent.  But we kill it first to prevent
				   it doing anything before we kill the whole
				   process in a moment.  We can use
				   PTRACE_KILL on a thread that's not already
				   stopped.  Then the value we pass in
				   PTRACE_DETACH just sets the death
				   signal reported to the real parent.  */
				ptrace(PTRACE_KILL, leader->pid, 0, 0);
				if (debug)
					fprintf(stderr,
						" [%d exit %d kills %d]\n",
						tcp->pid, sig, leader->pid);
				detach(leader, sig);
			}
			detach(tcp, sig);
		}
		else if (ptrace(PTRACE_CONT, tcp->pid, (char *) 1, sig) < 0) {
			perror("strace: ptrace(PTRACE_CONT, ...)");
			cleanup();
			return -1;
		}
		else {
			if (leader != NULL && leader != tcp)
				droptcb(tcp);
			/* The leader will report to us as parent now,
			   and then we'll get to the SIG==-1 case.  */
			return 0;
		}
	}

	/* Note that TCP and LEADER are no longer valid,
	   but we can still compare against them.  */
	if (leader != NULL) {
		unsigned int i;
		for (i = 0; i < tcbtabsize; i++) {
			struct tcb *t = tcbtab[i];
			if (t != tcp && (t->flags & TCB_CLONE_DETACHED)
			    && t->parent == leader)
				droptcb(t);
		}
	}

	return 0;
}
#endif

static int
trace()
{
	int pid;
	int wait_errno;
	int status;
	struct tcb *tcp;
#ifdef LINUX
	struct rusage ru;
#ifdef __WALL
	static int wait4_options = __WALL;
#endif
#endif /* LINUX */

	while (nprocs != 0) {
		if (interactive)
			sigprocmask(SIG_SETMASK, &empty_set, NULL);
#ifdef LINUX
#ifdef __WALL
		pid = wait4(-1, &status, wait4_options, cflag ? &ru : NULL);
		if (pid < 0 && (wait4_options & __WALL) && errno == EINVAL) {
			/* this kernel does not support __WALL */
			wait4_options &= ~__WALL;
			errno = 0;
			pid = wait4(-1, &status, wait4_options,
					cflag ? &ru : NULL);
		}
		if (pid < 0 && !(wait4_options & __WALL) && errno == ECHILD) {
			/* most likely a "cloned" process */
			pid = wait4(-1, &status, __WCLONE,
					cflag ? &ru : NULL);
			if (pid == -1) {
				fprintf(stderr, "strace: clone wait4 "
						"failed: %s\n", strerror(errno));
			}
		}
#else
		pid = wait4(-1, &status, 0, cflag ? &ru : NULL);
#endif /* __WALL */
#endif /* LINUX */
#ifdef SUNOS4
		pid = wait(&status);
#endif /* SUNOS4 */
		wait_errno = errno;
		if (interactive)
			sigprocmask(SIG_BLOCK, &blocked_set, NULL);

		if (interrupted)
			return 0;

		if (pid == -1) {
			switch (wait_errno) {
			case EINTR:
				continue;
			case ECHILD:
				/*
				 * We would like to verify this case
				 * but sometimes a race in Solbourne's
				 * version of SunOS sometimes reports
				 * ECHILD before sending us SIGCHILD.
				 */
#if 0
				if (nprocs == 0)
					return 0;
				fprintf(stderr, "strace: proc miscount\n");
				exit(1);
#endif
				return 0;
			default:
				errno = wait_errno;
				perror("strace: wait");
				return -1;
			}
		}
		if (debug)
			fprintf(stderr, " [wait(%#x) = %u]\n", status, pid);

		/* Look up `pid' in our table. */
		if ((tcp = pid2tcb(pid)) == NULL) {
#ifdef LINUX
			if (followfork || followvfork) {
				/* This is needed to go with the CLONE_PTRACE
				   changes in process.c/util.c: we might see
				   the child's initial trap before we see the
				   parent return from the clone syscall.
				   Leave the child suspended until the parent
				   returns from its system call.  Only then
				   will we have the association of parent and
				   child so that we know how to do clearbpt
				   in the child.  */
				if ((tcp = alloctcb(pid)) == NULL) {
					fprintf(stderr, " [tcb table full]\n");
					kill(pid, SIGKILL); /* XXX */
					return 0;
				}
				tcp->flags |= TCB_ATTACHED | TCB_SUSPENDED;
				newoutf(tcp);
				if (!qflag)
					fprintf(stderr, "\
Process %d attached (waiting for parent)\n",
						pid);
			}
			else
				/* This can happen if a clone call used
				   CLONE_PTRACE itself.  */
#endif
			{
				fprintf(stderr, "unknown pid: %u\n", pid);
				if (WIFSTOPPED(status))
					ptrace(PTRACE_CONT, pid, (char *) 1, 0);
				exit(1);
			}
		}
		/* set current output file */
		outf = tcp->outf;
		if (cflag) {
#ifdef LINUX
			tv_sub(&tcp->dtime, &ru.ru_stime, &tcp->stime);
			tcp->stime = ru.ru_stime;
#endif /* !LINUX */
		}

		if (tcp->flags & TCB_SUSPENDED) {
			/*
			 * Apparently, doing any ptrace() call on a stopped
			 * process, provokes the kernel to report the process
			 * status again on a subsequent wait(), even if the
			 * process has not been actually restarted.
			 * Since we have inspected the arguments of suspended
			 * processes we end up here testing for this case.
			 */
			continue;
		}
		if (WIFSIGNALED(status)) {
			if (!cflag
			    && (qual_flags[WTERMSIG(status)] & QUAL_SIGNAL)) {
				printleader(tcp);
				tprintf("+++ killed by %s +++",
					signame(WTERMSIG(status)));
				printtrailer(tcp);
			}
#ifdef TCB_GROUP_EXITING
			handle_group_exit(tcp, -1);
#else
			droptcb(tcp);
#endif
			continue;
		}
		if (WIFEXITED(status)) {
			if (debug)
				fprintf(stderr, "pid %u exited\n", pid);
			if (tcp->flags & TCB_ATTACHED)
				fprintf(stderr,
					"PANIC: attached pid %u exited\n",
					pid);
#ifdef TCB_GROUP_EXITING
			handle_group_exit(tcp, -1);
#else
			droptcb(tcp);
#endif
			continue;
		}
		if (!WIFSTOPPED(status)) {
			fprintf(stderr, "PANIC: pid %u not stopped\n", pid);
			droptcb(tcp);
			continue;
		}
		if (debug)
			fprintf(stderr, "pid %u stopped, [%s]\n",
				pid, signame(WSTOPSIG(status)));

		if (tcp->flags & TCB_STARTUP) {
			/*
			 * This flag is there to keep us in sync.
			 * Next time this process stops it should
			 * really be entering a system call.
			 */
			tcp->flags &= ~TCB_STARTUP;
			if (tcp->flags & TCB_ATTACHED) {
				/*
				 * Interestingly, the process may stop
				 * with STOPSIG equal to some other signal
				 * than SIGSTOP if we happend to attach
				 * just before the process takes a signal.
				 */
				if (!WIFSTOPPED(status)) {
					fprintf(stderr,
						"pid %u not stopped\n", pid);
					detach(tcp, WSTOPSIG(status));
					continue;
				}
			}
			else {
#ifdef SUNOS4
				/* A child of us stopped at exec */
				if (WSTOPSIG(status) == SIGTRAP && followvfork)
					fixvfork(tcp);
#endif /* SUNOS4 */
			}
			if (tcp->flags & TCB_BPTSET) {
				if (clearbpt(tcp) < 0) /* Pretty fatal */ {
					droptcb(tcp);
					cleanup();
					return -1;
				}
			}
			goto tracing;
		}

		if (WSTOPSIG(status) != SIGTRAP) {
			if (WSTOPSIG(status) == SIGSTOP &&
					(tcp->flags & TCB_SIGTRAPPED)) {
				/*
				 * Trapped attempt to block SIGTRAP
				 * Hope we are back in control now.
				 */
				tcp->flags &= ~(TCB_INSYSCALL | TCB_SIGTRAPPED);
				if (ptrace(PTRACE_SYSCALL,
						pid, (char *) 1, 0) < 0) {
					perror("trace: ptrace(PTRACE_SYSCALL, ...)");
					cleanup();
					return -1;
				}
				continue;
			}
			if (!cflag
			    && (qual_flags[WSTOPSIG(status)] & QUAL_SIGNAL)) {
				unsigned long addr = 0, pc = 0;
#ifdef PT_GETSIGINFO
#				define PSR_RI	41
				struct siginfo si;
				unsigned long psr;

				upeek(pid, PT_CR_IPSR, &psr);
				upeek(pid, PT_CR_IIP, &pc);

				pc += (psr >> PSR_RI) & 0x3;
				ptrace(PT_GETSIGINFO, pid, 0, (long) &si);
				addr = (unsigned long) si.si_addr;
#endif
				printleader(tcp);
				tprintf("--- %s (%s) @ %lx (%lx) ---",
					signame(WSTOPSIG(status)),
					strsignal(WSTOPSIG(status)), pc, addr);
				printtrailer(tcp);
			}
			if ((tcp->flags & TCB_ATTACHED) &&
				!sigishandled(tcp, WSTOPSIG(status))) {
#ifdef TCB_GROUP_EXITING
				handle_group_exit(tcp, WSTOPSIG(status));
#else
				detach(tcp, WSTOPSIG(status));
#endif
				continue;
			}
			if (ptrace(PTRACE_SYSCALL, pid, (char *) 1,
				   WSTOPSIG(status)) < 0) {
				perror("trace: ptrace(PTRACE_SYSCALL, ...)");
				cleanup();
				return -1;
			}
			tcp->flags &= ~TCB_SUSPENDED;
			continue;
		}
		if (trace_syscall(tcp) < 0) {
			if (tcp->flags & TCB_ATTACHED)
				detach(tcp, 0);
			else {
				ptrace(PTRACE_KILL,
					tcp->pid, (char *) 1, SIGTERM);
				droptcb(tcp);
			}
			continue;
		}
		if (tcp->flags & TCB_EXITING) {
#ifdef TCB_GROUP_EXITING
			if (tcp->flags & TCB_GROUP_EXITING) {
				if (handle_group_exit(tcp, 0) < 0)
					return -1;
				continue;
			}
#endif
			if (tcp->flags & TCB_ATTACHED)
				detach(tcp, 0);
			else if (ptrace(PTRACE_CONT, pid, (char *) 1, 0) < 0) {
				perror("strace: ptrace(PTRACE_CONT, ...)");
				cleanup();
				return -1;
			}
			continue;
		}
		if (tcp->flags & TCB_SUSPENDED) {
			if (!qflag)
				fprintf(stderr, "Process %u suspended\n", pid);
			continue;
		}
	tracing:
		if (ptrace(PTRACE_SYSCALL, pid, (char *) 1, 0) < 0) {
			perror("trace: ptrace(PTRACE_SYSCALL, ...)");
			cleanup();
			return -1;
		}
	}
	return 0;
}

#endif /* !USE_PROCFS */

static int curcol;

#ifdef __STDC__
#include <stdarg.h>
#define VA_START(a, b) va_start(a, b)
#else
#include <varargs.h>
#define VA_START(a, b) va_start(a)
#endif

void
#ifdef __STDC__
tprintf(const char *fmt, ...)
#else
tprintf(fmt, va_alist)
char *fmt;
va_dcl
#endif
{
	va_list args;

	VA_START(args, fmt);
	if (outf)
		curcol += vfprintf(outf, fmt, args);
	va_end(args);
	return;
}

void
printleader(tcp)
struct tcb *tcp;
{
	if (tcp_last && (!outfname || followfork < 2 || tcp_last == tcp)) {
		tcp_last->flags |= TCB_REPRINT;
		tprintf(" <unfinished ...>\n");
	}
	curcol = 0;
	if ((followfork == 1 || pflag_seen > 1) && outfname)
		tprintf("%-5d ", tcp->pid);
	else if (nprocs > 1 && !outfname)
		tprintf("[pid %5u] ", tcp->pid);
	if (tflag) {
		char str[sizeof("HH:MM:SS")];
		struct timeval tv, dtv;
		static struct timeval otv;

		gettimeofday(&tv, NULL);
		if (rflag) {
			if (otv.tv_sec == 0)
				otv = tv;
			tv_sub(&dtv, &tv, &otv);
			tprintf("%6ld.%06ld ",
				(long) dtv.tv_sec, (long) dtv.tv_usec);
			otv = tv;
		}
		else if (tflag > 2) {
			tprintf("%ld.%06ld ",
				(long) tv.tv_sec, (long) tv.tv_usec);
		}
		else {
			time_t local = tv.tv_sec;
			strftime(str, sizeof(str), "%T", localtime(&local));
			if (tflag > 1)
				tprintf("%s.%06ld ", str, (long) tv.tv_usec);
			else
				tprintf("%s ", str);
		}
	}
	if (iflag)
		printcall(tcp);
}

void
tabto(col)
int col;
{
	if (curcol < col)
		tprintf("%*s", col - curcol, "");
}

void
printtrailer(tcp)
struct tcb *tcp;
{
	tprintf("\n");
	tcp_last = NULL;
}

#ifdef HAVE_MP_PROCFS

int mp_ioctl (int fd, int cmd, void *arg, int size) {

	struct iovec iov[2];
	int n = 1;

	iov[0].iov_base = &cmd;
	iov[0].iov_len = sizeof cmd;
	if (arg) {
		++n;
		iov[1].iov_base = arg;
		iov[1].iov_len = size;
	}

	return writev (fd, iov, n);
}

#endif
