/*
 * Strauss awareness implementation.
 *
 * Copyright (c) 2018-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "strauss.h"

static const char *strauss[] = {
	"",
	"     ____",
	"    /    \\",
	"   |-. .-.|",
	"   (_@)(_@)",
	"   .---_  \\",
	"  /..   \\_/",
	"  |__.-^ /",
	"      }  |",
	"     |   [",
	"     [  ]",
	"    ]   |",
	"    |   [",
	"    [  ]",
	"   /   |        __",
	"  \\|   |/     _/ /_",
	" \\ |   |//___/__/__/_",
	"\\\\  \\ /  //    -____/_",
	"//   \"   \\\\      \\___.-",
	" //     \\\\  __.----._/_",
	"/ '/|||\\` .-         __>",
	"[        /         __.-",
	"[        [           }",
	"\\        \\          /",
	" \"-._____ \\.____.--\"",
	"    |  | |  |",
	"    |  | |  |",
	"    |  | |  |",
	"    |  | |  |",
	"    {  } {  }",
	"    |  | |  |",
	"    |  | |  |",
	"    |  | |  |",
	"    /  { |  |",
	" .-\"   / [   -._",
	"/___/ /   \\ \\___\"-.",
	"    -\"     \"-",
	};

const size_t strauss_lines = ARRAY_SIZE(strauss);

enum { MAX_TIP_LINES = 14 };

static const char *tips_tricks_tweaks[][MAX_TIP_LINES] = {
	{ "strace has an extensive manual page",
	  "that covers all the possible options",
	  "and contains several useful invocation",
	  "examples." },
	{ "You can use -o|COMMAND to redirect strace's",
	  "output to COMMAND.  This may be useful",
	  "in cases when there is a redirection",
	  "in place for the traced program.  Don't",
	  "forget to escape the pipe character, though,",
	  "as it is usually interpreted by the shell." },
	{ "It's possible to display timestamps",
	  " produced by -r, -t, and -T options",
	  "with nanosecond precision using their",
	  "long variants: --relative-timestamps=ns,",
	  "--absolute-timestamps=ns, and",
	  "--syscall-times=ns, respectively.", "",
	  "Why microsecond precision is the default?",
	  "To preserve the historic output format",
	  "which was limited by struct timeval",
	  "precision." },
	{ "A particular quote from a particular novel",
	  "by Arthur C. Clarke is printed if an attempt",
	  "is made to attach to a particular process." },
	{ "It's possible to tune the environment",
	  "of the traced process using the -E/--env",
	  "option:", "",
	  "    strace -E REMOVE_VAR -E VAR=new_val" },
#ifdef ENABLE_STACKTRACE
	{ "You can print a stack trace for each traced",
	  "call by specifying -k/--stack-traces option.",
# ifdef USE_DEMANGLE
	  "It can even demangle symbol names.",
# endif
	  },
#else
	{ "We wish we could tell you that you can",
	  "specify -k/--stack-traces option to print",
	  "stack traces for each traced system call,",
	  "but, unfortunately, you can't: this strace",
	  "binary is built without stack tracing",
	  "support." },
#endif
#ifdef ENABLE_SECONTEXT
	{ "You can print SELinux contexts associated",
	  "with PIDs, FDs, and paths by specifying",
	  "--secontext option.  Unless provided",
	  "with the \"full\" parameter, it prints only",
	  "SELinux context type, and the \"mismatch\"",
	  "parameter enables printing of the expected",
	  "context in case of mismatch, so", "",
	  "    strace --secontext=full,mismatch", "",
	  "will show all gory SELinux details." },
#else
	{ "We wish we could tell you that you can",
	  "specify --secontext option to print SELinux",
	  "context for each PID, FD, and path occurred",
	  "in the trace, but, unfortunately, you can't:",
	  "this strace binary is built without SELinux",
	  "support." },
#endif
	{ "Have you ever been bitten by an accidental",
	  "overwrite of the output file specified",
	  "in the -o option?  Specify",
	  "-A/--output-append-mode as well,",
	  "and this problem will never bite you again!" },
	{ "strace is about as old as the Linux kernel.",
	  "It has been originally written for SunOS",
	  "by Paul Kranenburg in 1991.  The support",
	  "for all OSes except Linux was dropped",
	  "in 2012, though, in strace 4.7." },
	{ "strace is able to decode netlink messages.",
	  "It does so automatically for I/O performed",
	  "on netlink sockets.  Try it yourself:", "",
	  "    strace -e%network ip a" },
	{ "Filtered syscalls, errors, and signals can",
	  "be specified either by name or by number,",
	  "for example:", "",
	  "    strace --trace=0,1,2 --signal=2,15 true" },
	{ "It is possible to specify -r and -t options",
	  "simultaneously since strace 4.22." },
	{ "Strace can print only successful syscall",
	  "invocations when supplied with",
	  "-z/--successful-only option.  There's also",
	  "a possibility to filter calls with other",
	  "statuses, please refer to -e status option",
	  "documentation." },
	{ "If you trace a process that uses KVM",
	  "subsystem, --kvm=vcpu option may be of use:",
	  "it prints KVM VCPU exit reason.  It requires",
	  "Linux 4.16+, however." },
	{ "It is possible to get strace out of your way",
	  "(in terms of parent/child relationships and",
	  "signal communication) with -D/--daemonize",
	  "option.  Another option that may be of use",
	  "in this case is -I/--interruptible, it",
	  "restricts the set of signals that interrupt",
	  "strace." },
	{ "If strace is too chatty to your taste, you",
	  "can silence it with -qqq option." },
	{ "strace prints file paths along with file",
	  "descriptor numbers when it is invoked with",
	  "-y/--decode-fds option.",
	  "When -yy (or --decode-fds=all) is provided,",
	  "it also prints protocol-specific information",
	  "for sockets and device numbers for character",
	  "and block device files." },
	{ "You can control what columns are shown",
	  "in the call summary table produced by -c/-C",
	  "options with -U/--summary-columns option.",
	  "It is a way to get minimum/maximum call",
	  "duration printed, for example:", "",
	  "    strace -c -U name,min-time,max-time ls" },
	{ "If you feel that syscall duration shown",
	  "in the call summary table (-c/-C option)",
	  "is not right, you can try to use -w option",
	  "(that collects wall clock time,",
	  "instead of system time), maybe that is what",
	  "you are looking for." },
	{ "strace understands -z option since 2002,",
	  "but it wasn't documented because its",
	  "implementation was broken.  Only 17 years",
	  "later, in strace 5.2, it was properly",
	  "implemented and documented." },
	{ "If you feel that strace is too slow, you may",
	  "want to try --seccomp-bpf option, maybe you",
	  "will feel better." },
	{ "-v is a shorthand for -e abbrev=none and not",
	  "for -e verbose=all.  It is idiosyncratic,",
	  "but it is the historic behaviour." },
	{ "strace uses netlink for printing",
	  "protocol-specific information about socket",
	  "descriptors (-yy option)." },
	{ "strace is able to tamper with tracees'",
	  "execution by injecting an arbitrary return",
	  "or error value instead of syscall execution,",
	  "for example:", "",
	  "    strace --inject=unlink:retval=0", "",
	  "will prevent execution of unlink calls, but",
	  "the traced process will think that the calls",
	  "have succeeded." },
	{ "strace's tampering capabilities include",
	  "injection of arbitrary return/error values,",
	  "injection of a signal, injection of a delay",
	  "or data before or after syscall execution." },
	{ "If you want to see numerical values of named",
	  "constants, there is an option for that:",
	  "-X/--const-print-style.  When -Xraw",
	  "(or --const-print-style=raw) is provided,",
	  "strace prints just the numerical value",
	  "of an argument; with -Xverbose, it prints",
	  "values in both numerical and symbolic form." },
	{ "getpid syscall is present on all",
	  "architectures except on Alpha, where getxpid",
	  "syscall (that returns a pair of PID and PPID",
	  "in a pair of registers) is used instead.",
	  "Other two examples of syscalls that utilise",
	  "two registers for their return values are",
	  "getxuid and getxgid: they return a pair",
	  "of real and effective UIDs/GIDs." },
	{ "There are three syscalls that implement",
	  "generic \"open file\" task: open, openat,",
	  "and openat2.  On some (newly supported)",
	  "architectures, open syscall is not even",
	  "present.  How to write a robust filtering",
	  "expression in this case?",
	  "With the conditional syntax, for example:", "",
	  "    strace --trace=?open,?openat,?openat2", "",
	  "You may want to escape question marks, since",
	  "your shell may interpret them as a path glob",
	  "expression." },
	{ "It is possible to use regular expressions",
	  "for syscall names in the -e trace",
	  "expression, for example:", "",
	  "    strace -e trace=/^sched_.*", "",
	  "will trace all scheduling-related syscalls." },
	{ "IA-64 (Itanium) uses syscall numbers",
	  "beginning from 1024, because numbers",
	  "beginning from 0 were designated for i386",
	  "compat layer (that has never been",
	  "upstreamed).  Another example",
	  "of an architecture with sparse syscall table",
	  "is MIPS, with parts of it beginning at index",
	  "0 (SVR4 ABI), 1000 (SysV ABI), 2000",
	  "(BSD 4.3 ABI), 3000 (POSIX ABI), 4000 (Linux",
	  "O32 ABI), 5000 (Linux N64 ABI), and 6000",
	  "(Linux N32 ABI)." },
	{ "Der Strauss, the strace's project mascot,",
	  "was conceived in 2017.  It is a brainchild",
	  "of Vitaly Chaykovsky." },
	/* https://github.com/strace/strace/issues/14 */
	{ "Medicinal effects of strace can be achieved",
	  "by invoking it with the following options:", "",
	  "    strace -DDDqqq -enone --signal=none" },
	{ "Historically, supplying -o option to strace",
	  "leads to silencing of messages about tracee",
	  "attach/detach and personality changes.",
	  "It can be now overridden with --quiet=none",
	  "option." },
	{ "You can avoid tracing of \"other programs\"",
	  "that are executed by the traced program",
	  "with -b execve option." },
	{ "-F option used to be a separate option",
	  "for following vfork calls." },
	{ "It is possible to provide multiple PIDs",
	  "to a single -p option with white space",
	  "or comma as accepted delimiter, in order",
	  "to support usage like", "",
	  "    strace -p \"`pidof PROG`\"",
	  "or",
	  "    strace -p \"`pgrep PROG`\"", "",
	  "pidof uses space as a delimiter, pgrep uses",
	  "newline." },
	{ "-n option, that prints syscall numbers,",
	  "while seemingly quite obvious functionality,",
	  "was added to strace only in version 5.9,",
	  "in the year 2020." },
	{ "Instead of tirelessly specifying",
	  "architecture- and libc-specific sets",
	  "of syscalls pertaining specific task each",
	  "time, one can try to use pre-defined syscall",
	  "classes.  For example,", "",
	  "    strace -e%creds", "",
	  "will trace all syscalls related to accessing",
	  "and modifying process's user/group IDs",
	  "and capability sets.  Other pre-defined",
	  "syscall classes include %clock, %desc,",
	  "%file, %ipc, %memory, %net, %process,",
	  "and %signal." },
	{ "Trying to figure out communication between",
	  "tracees inside a different PID namespace",
	  "(in so-called \"containers\", for example)?",
	  "Try out the --pidns-translation option,",
	  "it prints PIDs in strace's PID NS when a PID",
	  "reference from a different PID NS occurs",
	  "in trace.  It is not enabled by default",
	  "because there is no sane kernel API",
	  "to perform PID translation between",
	  "namespaces, so each such translation",
	  "requires many reads and ioctls in procfs,",
	  "which may incur severe performance penalty." },
	{ "If you don't like the way strace escapes",
	  "non-printable characters using octal",
	  "numbers, and don't want to sacrifice",
	  "readability of the ASCII output with -x/-xx",
	  "options, you might want to try", "",
	  "    strace --strings-in-hex=non-ascii-chars", "",
	  "that will change escape sequences",
	  "to hexadecimal numbers usage." },
	{ "-Y option (an alias to --decode-pids=comm)",
	  "shows comm string associated with the PID." },
	{ "Historically, strace had a mis-feature",
	  "of interpreting the \" (deleted)\" part",
	  "of the proc/pid/fd symlinks as a part",
	  "of the filename.  This peculiar behaviour",
	  "ended with strace 5.19, which also enables",
	  "path tracing to trace FDs associated",
	  "with specific paths even after the paths",
	  "are unlinked." },
	{ "It seems that IA-64, POWER and s390 are",
	  "the only architectures where it is possible",
	  "for strace to account for syscall time",
	  "properly by relying on the system time usage",
	  "reported by the kernel: these are the only",
	  "architectures that HAVE_VIRT_CPU_ACCOUNTING",
	  "config option enabled and thusly account",
	  "the CPU time on syscall entering and exiting",
	  "instead of approximating it." },
};

static const char tip_top[] =
	"  ______________________________________________    ";
static const char tip_bottom[] =
	" \\______________________________________________/   ";
static const char *tip_left[] = { " / ", " | "};
static const char *tip_right[] = {
	" \\   ",
	" |   ",
	" \\   ",
	"  \\  ",
	"  _\\ ",
	" /   ",
	" |   ", };

enum tips_fmt show_tips = TIPS_NONE;
int tip_id = TIP_ID_RANDOM;

void
print_strauss(size_t verbosity)
{
	if (verbosity < STRAUSS_START_VERBOSITY)
		return;

	verbosity = MIN(verbosity - STRAUSS_START_VERBOSITY, strauss_lines);

	for (size_t i = 0; i < verbosity; i++)
		puts(strauss[i]);
}

void
print_totd(void)
{
	static bool printed = false;
	const int w = (int) (sizeof(tip_top) - 1 - strlen(tip_left[0])
				- strlen(tip_right[0]));
	struct timeval tv;
	size_t id;
	size_t i;

	if (printed || show_tips == TIPS_NONE)
		return;

	if (tip_id == TIP_ID_RANDOM) {
		gettimeofday(&tv, NULL);
		srand(tv.tv_sec ^ tv.tv_usec);
		id = rand();
	} else {
		id = tip_id;
	}
	id %= ARRAY_SIZE(tips_tricks_tweaks);

	fprintf(stderr, "%s%s\n", tip_top, strauss[1]);
	fprintf(stderr, "%s%-*s%s%s\n",
		tip_left[0], w, "", tip_right[0], strauss[2]);
	for (i = 0; (i < MAX_TIP_LINES) && (tips_tricks_tweaks[id][i] ||
					    (i < (ARRAY_SIZE(tip_right) - 1)));
	     i++) {
		fprintf(stderr, "%s%-*s%s%s\n",
			tip_left[MIN(i + 1, ARRAY_SIZE(tip_left) - 1)],
			w, tips_tricks_tweaks[id][i] ?: "",
			tip_right[MIN(i + 1, ARRAY_SIZE(tip_right) - 1)],
			strauss[MIN(3 + i, strauss_lines - 1)]);
	}
	fprintf(stderr, "%s%s\n",
		tip_bottom, strauss[MIN(3 + i, strauss_lines - 1)]);
	do {
		fprintf(stderr, "%*s%*s%*s%s\n",
			(int) strlen(tip_left[0]), "",
			w, "",
			(int) strlen(tip_right[0]), "",
			strauss[MIN(4 + i, strauss_lines - 1)]);
	} while ((show_tips == TIPS_FULL) && (4 + ++i < strauss_lines));

	printed = true;
}
