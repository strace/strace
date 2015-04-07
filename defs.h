/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <features.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#endif
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#ifdef STDC_HEADERS
# include <stddef.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
/* Open-coding isprint(ch) et al proved more efficient than calling
 * generalized libc interface. We don't *want* to do non-ASCII anyway.
 */
/* #include <ctype.h> */
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>

#ifndef HAVE_STRERROR
const char *strerror(int);
#endif
#ifndef HAVE_STPCPY
/* Some libc have stpcpy, some don't. Sigh...
 * Roll our private implementation...
 */
#undef stpcpy
#define stpcpy strace_stpcpy
extern char *stpcpy(char *dst, const char *src);
#endif

#if defined __GNUC__ && defined __GNUC_MINOR__
# define GNUC_PREREQ(maj, min)	\
	((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define __attribute__(x)	/* empty */
# define GNUC_PREREQ(maj, min)	0
#endif

#if GNUC_PREREQ(2, 5)
# define ATTRIBUTE_NORETURN	__attribute__((__noreturn__))
#else
# define ATTRIBUTE_NORETURN	/* empty */
#endif

#if GNUC_PREREQ(2, 7)
# define ATTRIBUTE_FORMAT(args)	__attribute__((__format__ args))
# define ATTRIBUTE_ALIGNED(arg)	__attribute__((__aligned__(arg)))
# define ATTRIBUTE_PACKED	__attribute__((__packed__))
#else
# define ATTRIBUTE_FORMAT(args)	/* empty */
# define ATTRIBUTE_ALIGNED(arg)	/* empty */
# define ATTRIBUTE_PACKED	/* empty */
#endif

#if GNUC_PREREQ(3, 0)
# define ATTRIBUTE_MALLOC	__attribute__((__malloc__))
#else
# define ATTRIBUTE_MALLOC	/* empty */
#endif

#if GNUC_PREREQ(3, 1)
# define ATTRIBUTE_NOINLINE	__attribute__((__noinline__))
#else
# define ATTRIBUTE_NOINLINE	/* empty */
#endif

#if GNUC_PREREQ(4, 3)
# define ATTRIBUTE_ALLOC_SIZE(args)	__attribute__((__alloc_size__ args))
#else
# define ATTRIBUTE_ALLOC_SIZE(args)	/* empty */
#endif

#ifndef offsetof
# define offsetof(type, member)	\
	(((char *) &(((type *) NULL)->member)) - ((char *) (type *) NULL))
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* macros */
#ifndef MAX
# define MAX(a, b)		(((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
# define MIN(a, b)		(((a) < (b)) ? (a) : (b))
#endif
#define CLAMP(val, min, max) MIN(MAX(min, val), max)

/* Glibc has an efficient macro for sigemptyset
 * (it just does one or two assignments of 0 to internal vector of longs).
 */
#if defined(__GLIBC__) && defined(__sigemptyset) && !defined(sigemptyset)
# define sigemptyset __sigemptyset
#endif

/* Configuration section */
#ifndef DEFAULT_STRLEN
/* default maximum # of bytes printed in `printstr', change with -s switch */
# define DEFAULT_STRLEN	32
#endif
#ifndef DEFAULT_ACOLUMN
# define DEFAULT_ACOLUMN	40	/* default alignment column for results */
#endif
/*
 * Maximum number of args to a syscall.
 *
 * Make sure that all entries in all syscallent.h files have nargs <= MAX_ARGS!
 * linux/<ARCH>/syscallent*.h:
 * 	all have nargs <= 6 except mips o32 which has nargs <= 7.
 */
#ifndef MAX_ARGS
# ifdef LINUX_MIPSO32
#  define MAX_ARGS	7
# else
#  define MAX_ARGS	6
# endif
#endif
/* default sorting method for call profiling */
#ifndef DEFAULT_SORTBY
# define DEFAULT_SORTBY "time"
#endif
/*
 * Experimental code using PTRACE_SEIZE can be enabled here.
 * This needs Linux kernel 3.4.x or later to work.
 */
#define USE_SEIZE 1
/* To force NOMMU build, set to 1 */
#define NOMMU_SYSTEM 0
/*
 * Set to 1 to use speed-optimized vfprintf implementation.
 * It results in strace using about 5% less CPU in user space
 * (compared to glibc version).
 * But strace spends a lot of time in kernel space,
 * so overall it does not appear to be a significant win.
 * Thus disabled by default.
 */
#define USE_CUSTOM_PRINTF 0

#ifndef ERESTARTSYS
# define ERESTARTSYS    512
#endif
#ifndef ERESTARTNOINTR
# define ERESTARTNOINTR 513
#endif
#ifndef ERESTARTNOHAND
# define ERESTARTNOHAND 514
#endif
#ifndef ERESTART_RESTARTBLOCK
# define ERESTART_RESTARTBLOCK 516
#endif

#if defined(SPARC) || defined(SPARC64)
# define PERSONALITY0_WORDSIZE 4
# if defined(SPARC64)
#  define SUPPORTED_PERSONALITIES 2
#  define PERSONALITY1_WORDSIZE 8
# endif
#endif

#ifdef X86_64
# define SUPPORTED_PERSONALITIES 3
# define PERSONALITY0_WORDSIZE 8
# define PERSONALITY1_WORDSIZE 4
# define PERSONALITY2_WORDSIZE 4
#endif

#ifdef X32
# define SUPPORTED_PERSONALITIES 2
# define PERSONALITY0_WORDSIZE 4
# define PERSONALITY1_WORDSIZE 4
#endif

#ifdef ARM
/* one personality */
#endif

#ifdef AARCH64
/* The existing ARM personality, then AArch64 */
# define SUPPORTED_PERSONALITIES 2
# define PERSONALITY0_WORDSIZE 4
# define PERSONALITY1_WORDSIZE 8
# define DEFAULT_PERSONALITY 1
#endif

#ifdef POWERPC64
# define SUPPORTED_PERSONALITIES 2
# define PERSONALITY0_WORDSIZE 8
# define PERSONALITY1_WORDSIZE 4
#endif

#ifdef TILE
# define SUPPORTED_PERSONALITIES 2
# define PERSONALITY0_WORDSIZE 8
# define PERSONALITY1_WORDSIZE 4
# ifdef __tilepro__
#  define DEFAULT_PERSONALITY 1
# endif
#endif

#ifndef SUPPORTED_PERSONALITIES
# define SUPPORTED_PERSONALITIES 1
#endif
#ifndef DEFAULT_PERSONALITY
# define DEFAULT_PERSONALITY 0
#endif
#ifndef PERSONALITY0_WORDSIZE
# define PERSONALITY0_WORDSIZE SIZEOF_LONG
#endif

typedef struct sysent {
	unsigned nargs;
	int	sys_flags;
	int	(*sys_func)();
	const char *sys_name;
} struct_sysent;

typedef struct ioctlent {
	const char *symbol;
	unsigned int code;
} struct_ioctlent;

/* Trace Control Block */
struct tcb {
	int flags;		/* See below for TCB_ values */
	int pid;		/* If 0, this tcb is free */
	int qual_flg;		/* qual_flags[scno] or DEFAULT_QUAL_FLAGS + RAW */
	int u_error;		/* Error code */
	long scno;		/* System call number */
	long u_arg[MAX_ARGS];	/* System call arguments */
#if defined(LINUX_MIPSN32) || defined(X32)
	long long ext_arg[MAX_ARGS];
	long long u_lrval;	/* long long return value */
#endif
	long u_rval;		/* Return value */
#if SUPPORTED_PERSONALITIES > 1
	unsigned int currpers;	/* Personality at the time of scno update */
#endif
	int curcol;		/* Output column for this process */
	FILE *outf;		/* Output file for this process */
	const char *auxstr;	/* Auxiliary info from syscall (see RVAL_STR) */
	const struct_sysent *s_ent; /* sysent[scno] or dummy struct for bad scno */
	const struct_sysent *s_prev_ent; /* for "resuming interrupted SYSCALL" msg */
	struct timeval stime;	/* System time usage as of last process wait */
	struct timeval dtime;	/* Delta for system time usage */
	struct timeval etime;	/* Syscall entry time */

#ifdef USE_LIBUNWIND
	struct UPT_info* libunwind_ui;
	struct mmap_cache_t* mmap_cache;
	unsigned int mmap_cache_size;
	unsigned int mmap_cache_generation;
	struct queue_t* queue;
#endif
};

/* TCB flags */
/* We have attached to this process, but did not see it stopping yet */
#define TCB_STARTUP		0x01
#define TCB_IGNORE_ONE_SIGSTOP	0x02	/* Next SIGSTOP is to be ignored */
/*
 * Are we in system call entry or in syscall exit?
 *
 * This bit is set after all syscall entry processing is done.
 * Therefore, this bit will be set when next ptrace stop occurs,
 * which should be syscall exit stop. Other stops which are possible
 * directly after syscall entry (death, ptrace event stop)
 * are simpler and handled without calling trace_syscall(), therefore
 * the places where TCB_INSYSCALL can be set but we aren't in syscall stop
 * are limited to trace(), this condition is never observed in trace_syscall()
 * and below.
 * The bit is cleared after all syscall exit processing is done.
 *
 * Use entering(tcp) / exiting(tcp) to check this bit to make code more readable.
 */
#define TCB_INSYSCALL	0x04
#define TCB_ATTACHED	0x08	/* We attached to it already */
#define TCB_REPRINT	0x10	/* We should reprint this syscall on exit */
#define TCB_FILTERED	0x20	/* This system call has been filtered out */

/* qualifier flags */
#define QUAL_TRACE	0x001	/* this system call should be traced */
#define QUAL_ABBREV	0x002	/* abbreviate the structures of this syscall */
#define QUAL_VERBOSE	0x004	/* decode the structures of this syscall */
#define QUAL_RAW	0x008	/* print all args in hex for this syscall */
#define QUAL_SIGNAL	0x010	/* report events with this signal */
#define QUAL_READ	0x020	/* dump data read on this file descriptor */
#define QUAL_WRITE	0x040	/* dump data written to this file descriptor */
typedef uint8_t qualbits_t;
#define UNDEFINED_SCNO	0x100	/* Used only in tcp->qual_flg */

#define DEFAULT_QUAL_FLAGS (QUAL_TRACE | QUAL_ABBREV | QUAL_VERBOSE)

#define entering(tcp)	(!((tcp)->flags & TCB_INSYSCALL))
#define exiting(tcp)	((tcp)->flags & TCB_INSYSCALL)
#define syserror(tcp)	((tcp)->u_error != 0)
#define verbose(tcp)	((tcp)->qual_flg & QUAL_VERBOSE)
#define abbrev(tcp)	((tcp)->qual_flg & QUAL_ABBREV)
#define filtered(tcp)	((tcp)->flags & TCB_FILTERED)

struct xlat {
	unsigned int val;
	const char *str;
};
#define XLAT(x) { x, #x }
#define XLAT_END { 0, NULL }

extern const struct xlat addrfams[];
extern const struct xlat at_flags[];
extern const struct xlat open_access_modes[];
extern const struct xlat open_mode_flags[];
extern const struct xlat whence_codes[];

/* Format of syscall return values */
#define RVAL_DECIMAL	000	/* decimal format */
#define RVAL_HEX	001	/* hex format */
#define RVAL_OCTAL	002	/* octal format */
#define RVAL_UDECIMAL	003	/* unsigned decimal format */
#if defined(LINUX_MIPSN32) || defined(X32)
# if 0 /* unused so far */
#  define RVAL_LDECIMAL	004	/* long decimal format */
#  define RVAL_LHEX	005	/* long hex format */
#  define RVAL_LOCTAL	006	/* long octal format */
# endif
# define RVAL_LUDECIMAL	007	/* long unsigned decimal format */
#endif
#define RVAL_FD		010	/* file descriptor */
#define RVAL_MASK	017	/* mask for these values */

#define RVAL_STR	020	/* Print `auxstr' field after return val */
#define RVAL_NONE	040	/* Print nothing */

#define TRACE_FILE	001	/* Trace file-related syscalls. */
#define TRACE_IPC	002	/* Trace IPC-related syscalls. */
#define TRACE_NETWORK	004	/* Trace network-related syscalls. */
#define TRACE_PROCESS	010	/* Trace process-related syscalls. */
#define TRACE_SIGNAL	020	/* Trace signal-related syscalls. */
#define TRACE_DESC	040	/* Trace file descriptor-related syscalls. */
#define TRACE_MEMORY	0100	/* Trace memory mapping-related syscalls. */
#define SYSCALL_NEVER_FAILS	0200	/* Syscall is always successful. */
#define STACKTRACE_INVALIDATE_CACHE 0400  /* Trigger proc/maps cache updating */
#define STACKTRACE_CAPTURE_ON_ENTER 01000 /* Capture stacktrace on "entering" stage */
#define TRACE_INDIRECT_SUBCALL	02000	/* Syscall is an indirect socket/ipc subcall. */

#if defined(ARM) || defined(AARCH64) \
 || defined(I386) || defined(X32) || defined(X86_64) \
 || defined(IA64) \
 || defined(BFIN) \
 || defined(M68K) \
 || defined(MICROBLAZE) \
 || defined(S390) \
 || defined(SH) || defined(SH64) \
 || defined(SPARC) || defined(SPARC64) \
 /**/
# define NEED_UID16_PARSERS 1
#else
# define NEED_UID16_PARSERS 0
#endif

typedef enum {
	CFLAG_NONE = 0,
	CFLAG_ONLY_STATS,
	CFLAG_BOTH
} cflag_t;
extern cflag_t cflag;
extern bool debug_flag;
extern bool Tflag;
extern bool iflag;
extern bool count_wallclock;
extern unsigned int qflag;
extern bool not_failing_only;
extern unsigned int show_fd_path;
extern bool hide_log_until_execve;
/* are we filtering traces based on paths? */
extern const char **paths_selected;
#define tracing_paths (paths_selected != NULL)
extern unsigned xflag;
extern unsigned followfork;
#ifdef USE_LIBUNWIND
/* if this is true do the stack trace for every system call */
extern bool stack_trace_enabled;
#endif
extern unsigned ptrace_setoptions;
extern unsigned max_strlen;
extern unsigned os_release;
#undef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))

enum bitness_t { BITNESS_CURRENT = 0, BITNESS_32 };

void error_msg(const char *fmt, ...) ATTRIBUTE_FORMAT((printf, 1, 2));
void perror_msg(const char *fmt, ...) ATTRIBUTE_FORMAT((printf, 1, 2));
void error_msg_and_die(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
void perror_msg_and_die(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
void die_out_of_memory(void) ATTRIBUTE_NORETURN;

#if USE_CUSTOM_PRINTF
/*
 * See comment in vsprintf.c for allowed formats.
 * Short version: %h[h]u, %zu, %tu are not allowed, use %[l[l]]u.
 */
int strace_vfprintf(FILE *fp, const char *fmt, va_list args);
#else
# define strace_vfprintf vfprintf
#endif

extern void set_sortby(const char *);
extern void set_overhead(int);
extern void qualify(const char *);
extern void print_pc(struct tcb *);
extern int trace_syscall(struct tcb *);
extern void count_syscall(struct tcb *, const struct timeval *);
extern void call_summary(FILE *);

extern void clear_regs(void);
extern void get_regs(pid_t pid);
extern int get_scno(struct tcb *tcp);

extern int umoven(struct tcb *, long, unsigned int, void *);
#define umove(pid, addr, objp)	\
	umoven((pid), (addr), sizeof(*(objp)), (void *) (objp))
extern int umovestr(struct tcb *, long, unsigned int, char *);
extern int upeek(int pid, long, long *);

#if defined ALPHA || defined IA64 || defined MIPS \
 || defined SH || defined SPARC || defined SPARC64
# define HAVE_GETRVAL2
extern long getrval2(struct tcb *);
#else
# undef HAVE_GETRVAL2
#endif

extern const char *signame(const int);
extern void pathtrace_select(const char *);
extern int pathtrace_match(struct tcb *);
extern int getfdpath(struct tcb *, int, char *, unsigned);

extern const char *xlookup(const struct xlat *, const unsigned int);
extern const char *xlat_search(const struct xlat *, const size_t, const unsigned int);

extern unsigned long get_pagesize(void);
extern int string_to_uint(const char *str);
extern int next_set_bit(const void *bit_array, unsigned cur_bit, unsigned size_bits);

#define QUOTE_0_TERMINATED			0x01
#define QUOTE_OMIT_LEADING_TRAILING_QUOTES	0x02

extern int print_quoted_string(const char *, unsigned int, unsigned int);

/* a refers to the lower numbered u_arg,
 * b refers to the higher numbered u_arg
 */
#if HAVE_LITTLE_ENDIAN_LONG_LONG
# define LONG_LONG(a,b) \
	((long long)((unsigned long long)(unsigned)(a) | ((unsigned long long)(b)<<32)))
#else
# define LONG_LONG(a,b) \
	((long long)((unsigned long long)(unsigned)(b) | ((unsigned long long)(a)<<32)))
#endif
extern int getllval(struct tcb *, unsigned long long *, int);
extern int printllval(struct tcb *, const char *, int)
	ATTRIBUTE_FORMAT((printf, 2, 0));

extern void printxval(const struct xlat *, const unsigned int, const char *);
extern int printargs(struct tcb *);
extern int printargs_lu(struct tcb *);
extern int printargs_ld(struct tcb *);
extern void addflags(const struct xlat *, int);
extern int printflags(const struct xlat *, int, const char *);
extern const char *sprintflags(const char *, const struct xlat *, int);
extern const char *sprintmode(int);
extern const char *sprinttime(time_t);
extern void dumpiov_in_msghdr(struct tcb *, long);
extern void dumpiov_in_mmsghdr(struct tcb *, long);
extern void dumpiov(struct tcb *, int, long);
extern void dumpstr(struct tcb *, long, int);
extern void printstr(struct tcb *, long, long);
extern void printnum_int(struct tcb *, long, const char *)
	ATTRIBUTE_FORMAT((printf, 3, 0));
extern void printnum_long(struct tcb *, long, const char *)
	ATTRIBUTE_FORMAT((printf, 3, 0));
extern void printpath(struct tcb *, long);
extern void printpathn(struct tcb *, long, unsigned int);
#define TIMESPEC_TEXT_BUFSIZE (sizeof(long)*3 * 2 + sizeof("{%u, %u}"))
#define TIMEVAL_TEXT_BUFSIZE  TIMESPEC_TEXT_BUFSIZE
extern void printtv_bitness(struct tcb *, long, enum bitness_t, int);
#define printtv(tcp, addr)	\
	printtv_bitness((tcp), (addr), BITNESS_CURRENT, 0)
#define printtv_special(tcp, addr)	\
	printtv_bitness((tcp), (addr), BITNESS_CURRENT, 1)
extern char *sprinttv(char *, struct tcb *, long, enum bitness_t, int special);
extern void print_timespec(struct tcb *, long);
extern void sprint_timespec(char *, struct tcb *, long);
extern void printsiginfo(const siginfo_t *, bool);
extern void printsiginfo_at(struct tcb *tcp, long addr);
extern void printfd(struct tcb *, int);
extern bool print_sockaddr_by_inode(const unsigned long, const char *);
extern void print_dirfd(struct tcb *, int);
extern void printsock(struct tcb *, long, int);
extern void print_sock_optmgmt(struct tcb *, long, int);
extern void printrusage(struct tcb *, long);
#ifdef ALPHA
extern void printrusage32(struct tcb *, long);
#endif
extern void printuid(const char *, const unsigned int);
extern void print_sigset_addr_len(struct tcb *, long, long);
extern const char *sprintsigmask_n(const char *, const void *, unsigned int);
#define tprintsigmask_addr(prefix, mask) \
	tprints(sprintsigmask_n((prefix), (mask), sizeof(mask)))
extern void printsignal(int);
extern void tprint_iov(struct tcb *, unsigned long, unsigned long, int decode_iov);
extern void tprint_iov_upto(struct tcb *, unsigned long, unsigned long, int decode_iov, unsigned long);
extern void tprint_open_modes(int);
extern const char *sprint_open_modes(int);
extern void print_loff_t(struct tcb *, long);
extern void print_seccomp_filter(struct tcb *tcp, unsigned long);

extern const struct_ioctlent *ioctl_lookup(const unsigned int);
extern const struct_ioctlent *ioctl_next_match(const struct_ioctlent *);
extern void ioctl_print_code(const unsigned int);
extern int ioctl_decode(struct tcb *, const unsigned int, long);
extern int ioctl_decode_command_number(const unsigned int);
extern int block_ioctl(struct tcb *, const unsigned int, long);
extern int evdev_ioctl(struct tcb *, const unsigned int, long);
extern int loop_ioctl(struct tcb *, const unsigned int, long);
extern int mtd_ioctl(struct tcb *, const unsigned int, long);
extern int ptp_ioctl(struct tcb *, const unsigned int, long);
extern int rtc_ioctl(struct tcb *, const unsigned int, long);
extern int scsi_ioctl(struct tcb *, const unsigned int, long);
extern int sock_ioctl(struct tcb *, const unsigned int, long);
extern int term_ioctl(struct tcb *, const unsigned int, long);
extern int ubi_ioctl(struct tcb *, const unsigned int, long);
extern int v4l2_ioctl(struct tcb *, const unsigned int, long);

extern int tv_nz(const struct timeval *);
extern int tv_cmp(const struct timeval *, const struct timeval *);
extern double tv_float(const struct timeval *);
extern void tv_add(struct timeval *, const struct timeval *, const struct timeval *);
extern void tv_sub(struct timeval *, const struct timeval *, const struct timeval *);
extern void tv_mul(struct timeval *, const struct timeval *, int);
extern void tv_div(struct timeval *, const struct timeval *, int);

#ifdef USE_LIBUNWIND
extern void unwind_init(void);
extern void unwind_tcb_init(struct tcb *tcp);
extern void unwind_tcb_fin(struct tcb *tcp);
extern void unwind_cache_invalidate(struct tcb* tcp);
extern void unwind_print_stacktrace(struct tcb* tcp);
extern void unwind_capture_stacktrace(struct tcb* tcp);
#endif

/* Strace log generation machinery.
 *
 * printing_tcp: tcb which has incomplete line being printed right now.
 * NULL if last line has been completed ('\n'-terminated).
 * printleader(tcp) examines it, finishes incomplete line if needed,
 * the sets it to tcp.
 * line_ended() clears printing_tcp and resets ->curcol = 0.
 * tcp->curcol == 0 check is also used to detect completeness
 * of last line, since in -ff mode just checking printing_tcp for NULL
 * is not enough.
 *
 * If you change this code, test log generation in both -f and -ff modes
 * using:
 * strace -oLOG -f[f] test/threaded_execve
 * strace -oLOG -f[f] test/sigkill_rain
 * strace -oLOG -f[f] -p "`pidof web_browser`"
 */
extern struct tcb *printing_tcp;
extern void printleader(struct tcb *);
extern void line_ended(void);
extern void tabto(void);
extern void tprintf(const char *fmt, ...) ATTRIBUTE_FORMAT((printf, 1, 2));
extern void tprints(const char *str);

#if SUPPORTED_PERSONALITIES > 1
extern void set_personality(int personality);
extern unsigned current_personality;
#else
# define set_personality(personality) ((void)0)
# define current_personality 0
#endif

#if SUPPORTED_PERSONALITIES == 1
# define current_wordsize PERSONALITY0_WORDSIZE
#else
# if SUPPORTED_PERSONALITIES == 2 && PERSONALITY0_WORDSIZE == PERSONALITY1_WORDSIZE
#  define current_wordsize PERSONALITY0_WORDSIZE
# else
extern unsigned current_wordsize;
# endif
#endif

/* In many, many places we play fast and loose and use
 * tprintf("%d", (int) tcp->u_arg[N]) to print fds, pids etc.
 * We probably need to use widen_to_long() instead:
 */
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
# define widen_to_long(v) (current_wordsize == 4 ? (long)(int32_t)(v) : (long)(v))
#else
# define widen_to_long(v) ((long)(v))
#endif

extern const struct_sysent sysent0[];
extern const char *const errnoent0[];
extern const char *const signalent0[];
extern const struct_ioctlent ioctlent0[];
extern qualbits_t *qual_vec[SUPPORTED_PERSONALITIES];
#define qual_flags (qual_vec[current_personality])
#if SUPPORTED_PERSONALITIES > 1
extern const struct_sysent *sysent;
extern const char *const *errnoent;
extern const char *const *signalent;
extern const struct_ioctlent *ioctlent;
#else
# define sysent     sysent0
# define errnoent   errnoent0
# define signalent  signalent0
# define ioctlent   ioctlent0
#endif
extern unsigned nsyscalls;
extern unsigned nerrnos;
extern unsigned nsignals;
extern unsigned nioctlents;
extern unsigned num_quals;

/*
 * If you need non-NULL sysent[scno].sys_func and sysent[scno].sys_name
 */
#define SCNO_IS_VALID(scno) \
	((unsigned long)(scno) < nsyscalls && sysent[scno].sys_func)

/* Only ensures that sysent[scno] isn't out of range */
#define SCNO_IN_RANGE(scno) \
	((unsigned long)(scno) < nsyscalls)

#ifndef SYS_FUNC_NAME
# define SYS_FUNC_NAME(syscall_name) sys_ ## syscall_name
#endif

#define SYS_FUNC(syscall_name) int SYS_FUNC_NAME(syscall_name)(struct tcb *tcp)
