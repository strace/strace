/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (C) 2001-2017 The strace developers.
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

#ifndef STRACE_DEFS_H
#define STRACE_DEFS_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <features.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
/* Open-coding isprint(ch) et al proved more efficient than calling
 * generalized libc interface. We don't *want* to do non-ASCII anyway.
 */
/* #include <ctype.h> */
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "kernel_types.h"
#include "mpers_type.h"
#include "gcc_compat.h"
#include "sysent.h"

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

#ifndef offsetofend
# define offsetofend(type, member) \
	(offsetof(type, member) + sizeof(((type *)NULL)->member))
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]) + MUST_BE_ARRAY(a))

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

#if defined X86_64
# define SUPPORTED_PERSONALITIES 3
# define PERSONALITY2_WORDSIZE  4
# define PERSONALITY2_KLONGSIZE PERSONALITY0_KLONGSIZE
#elif defined AARCH64 \
   || defined POWERPC64 \
   || defined RISCV \
   || defined SPARC64 \
   || defined TILE \
   || defined X32
# define SUPPORTED_PERSONALITIES 2
#else
# define SUPPORTED_PERSONALITIES 1
#endif

#if defined TILE && defined __tilepro__
# define DEFAULT_PERSONALITY 1
#else
# define DEFAULT_PERSONALITY 0
#endif

#define PERSONALITY0_WORDSIZE  SIZEOF_LONG
#define PERSONALITY0_KLONGSIZE SIZEOF_KERNEL_LONG_T
#define PERSONALITY0_INCLUDE_PRINTERS_DECLS "native_printer_decls.h"
#define PERSONALITY0_INCLUDE_PRINTERS_DEFS "native_printer_defs.h"

#if SUPPORTED_PERSONALITIES > 1
# define PERSONALITY1_WORDSIZE  4
# define PERSONALITY1_KLONGSIZE PERSONALITY1_WORDSIZE
#endif

#if SUPPORTED_PERSONALITIES > 1 && defined HAVE_M32_MPERS
# define PERSONALITY1_INCLUDE_PRINTERS_DECLS "m32_printer_decls.h"
# define PERSONALITY1_INCLUDE_PRINTERS_DEFS "m32_printer_defs.h"
# define PERSONALITY1_INCLUDE_FUNCS "m32_funcs.h"
# define MPERS_m32_IOCTL_MACROS "ioctl_redefs1.h"
#else
# define PERSONALITY1_INCLUDE_PRINTERS_DECLS "native_printer_decls.h"
# define PERSONALITY1_INCLUDE_PRINTERS_DEFS "native_printer_defs.h"
# define PERSONALITY1_INCLUDE_FUNCS "empty.h"
#endif

#if SUPPORTED_PERSONALITIES > 2 && defined HAVE_MX32_MPERS
# define PERSONALITY2_INCLUDE_FUNCS "mx32_funcs.h"
# define PERSONALITY2_INCLUDE_PRINTERS_DECLS "mx32_printer_decls.h"
# define PERSONALITY2_INCLUDE_PRINTERS_DEFS "mx32_printer_defs.h"
# define MPERS_mx32_IOCTL_MACROS "ioctl_redefs2.h"
#else
# define PERSONALITY2_INCLUDE_PRINTERS_DECLS "native_printer_decls.h"
# define PERSONALITY2_INCLUDE_PRINTERS_DEFS "native_printer_defs.h"
# define PERSONALITY2_INCLUDE_FUNCS "empty.h"
#endif

typedef struct ioctlent {
	const char *symbol;
	unsigned int code;
} struct_ioctlent;

struct inject_opts {
	uint16_t first;
	uint16_t step;
	uint16_t signo;
	int rval;
};

#define MAX_ERRNO_VALUE			4095
#define INJECT_OPTS_RVAL_DEFAULT	(-(MAX_ERRNO_VALUE + 1))

/* Trace Control Block */
struct tcb {
	int flags;		/* See below for TCB_ values */
	int pid;		/* If 0, this tcb is free */
	int qual_flg;		/* qual_flags[scno] or DEFAULT_QUAL_FLAGS + RAW */
	unsigned long u_error;	/* Error code */
	kernel_ulong_t scno;	/* System call number */
	kernel_ulong_t u_arg[MAX_ARGS];	/* System call arguments */
	kernel_long_t u_rval;	/* Return value */
#if SUPPORTED_PERSONALITIES > 1
	unsigned int currpers;	/* Personality at the time of scno update */
#endif
	int sys_func_rval;	/* Syscall entry parser's return value */
	int curcol;		/* Output column for this process */
	FILE *outf;		/* Output file for this process */
	const char *auxstr;	/* Auxiliary info from syscall (see RVAL_STR) */
	void *_priv_data;	/* Private data for syscall decoding functions */
	void (*_free_priv_data)(void *); /* Callback for freeing priv_data */
	const struct_sysent *s_ent; /* sysent[scno] or dummy struct for bad scno */
	const struct_sysent *s_prev_ent; /* for "resuming interrupted SYSCALL" msg */
	struct inject_opts *inject_vec[SUPPORTED_PERSONALITIES];
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
#define TCB_TAMPERED	0x40	/* A syscall has been tampered with */
#define TCB_HIDE_LOG	0x80	/* We should hide everything (until execve) */
#define TCB_SKIP_DETACH_ON_FIRST_EXEC	0x100	/* -b execve should skip detach on first execve */

/* qualifier flags */
#define QUAL_TRACE	0x001	/* this system call should be traced */
#define QUAL_ABBREV	0x002	/* abbreviate the structures of this syscall */
#define QUAL_VERBOSE	0x004	/* decode the structures of this syscall */
#define QUAL_RAW	0x008	/* print all args in hex for this syscall */
#define QUAL_INJECT	0x010	/* tamper with this system call on purpose */
#define QUAL_SIGNAL	0x100	/* report events with this signal */
#define QUAL_READ	0x200	/* dump data read from this file descriptor */
#define QUAL_WRITE	0x400	/* dump data written to this file descriptor */

#define DEFAULT_QUAL_FLAGS (QUAL_TRACE | QUAL_ABBREV | QUAL_VERBOSE)

#define entering(tcp)	(!((tcp)->flags & TCB_INSYSCALL))
#define exiting(tcp)	((tcp)->flags & TCB_INSYSCALL)
#define syserror(tcp)	((tcp)->u_error != 0)
#define verbose(tcp)	((tcp)->qual_flg & QUAL_VERBOSE)
#define abbrev(tcp)	((tcp)->qual_flg & QUAL_ABBREV)
#define filtered(tcp)	((tcp)->flags & TCB_FILTERED)
#define hide_log(tcp)	((tcp)->flags & TCB_HIDE_LOG)

#include "xlat.h"

extern const struct xlat addrfams[];
extern const struct xlat at_flags[];
extern const struct xlat clocknames[];
extern const struct xlat dirent_types[];
extern const struct xlat evdev_abs[];
extern const struct xlat msg_flags[];
extern const struct xlat netlink_protocols[];
extern const struct xlat open_access_modes[];
extern const struct xlat open_mode_flags[];
extern const struct xlat resource_flags[];
extern const struct xlat setns_types[];
extern const struct xlat sg_io_info[];
extern const struct xlat socketlayers[];
extern const struct xlat whence_codes[];

/* Format of syscall return values */
#define RVAL_DECIMAL	000	/* decimal format */
#define RVAL_HEX	001	/* hex format */
#define RVAL_OCTAL	002	/* octal format */
#define RVAL_UDECIMAL	003	/* unsigned decimal format */
#define RVAL_FD		010	/* file descriptor */
#define RVAL_MASK	013	/* mask for these values */

#define RVAL_STR	020	/* Print `auxstr' field after return val */
#define RVAL_NONE	040	/* Print nothing */

#define RVAL_DECODED	0100	/* syscall decoding finished */

#define IOCTL_NUMBER_UNKNOWN 0
#define IOCTL_NUMBER_HANDLED 1
#define IOCTL_NUMBER_STOP_LOOKUP 010

#define indirect_ipccall(tcp) (tcp->s_ent->sys_flags & TRACE_INDIRECT_SUBCALL)

#if defined(ARM) || defined(AARCH64) \
 || defined(I386) || defined(X32) || defined(X86_64) \
 || defined(IA64) \
 || defined(BFIN) \
 || defined(M68K) \
 || defined(MICROBLAZE) \
 || defined(RISCV) \
 || defined(S390) \
 || defined(SH) || defined(SH64) \
 || defined(SPARC) || defined(SPARC64) \
 /**/
# define NEED_UID16_PARSERS 1
#else
# define NEED_UID16_PARSERS 0
#endif

enum sock_proto {
	SOCK_PROTO_UNKNOWN,
	SOCK_PROTO_UNIX,
	SOCK_PROTO_TCP,
	SOCK_PROTO_UDP,
	SOCK_PROTO_TCPv6,
	SOCK_PROTO_UDPv6,
	SOCK_PROTO_NETLINK
};
extern enum sock_proto get_proto_by_name(const char *);

enum iov_decode {
	IOV_DECODE_ADDR,
	IOV_DECODE_STR,
	IOV_DECODE_NETLINK
};

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

void error_msg(const char *fmt, ...) ATTRIBUTE_FORMAT((printf, 1, 2));
void perror_msg(const char *fmt, ...) ATTRIBUTE_FORMAT((printf, 1, 2));
void error_msg_and_die(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
void error_msg_and_help(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
void perror_msg_and_die(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_NORETURN;
void die_out_of_memory(void) ATTRIBUTE_NORETURN;

void *xmalloc(size_t size) ATTRIBUTE_MALLOC ATTRIBUTE_ALLOC_SIZE((1));
void *xcalloc(size_t nmemb, size_t size)
	ATTRIBUTE_MALLOC ATTRIBUTE_ALLOC_SIZE((1, 2));
void *xreallocarray(void *ptr, size_t nmemb, size_t size)
	ATTRIBUTE_ALLOC_SIZE((2, 3));
char *xstrdup(const char *str) ATTRIBUTE_MALLOC;

extern int read_int_from_file(const char *, int *);

extern void set_sortby(const char *);
extern void set_overhead(int);
extern void print_pc(struct tcb *);
extern int trace_syscall(struct tcb *, unsigned int *);
extern void count_syscall(struct tcb *, const struct timeval *);
extern void call_summary(FILE *);

extern void clear_regs(void);
extern int get_scno(struct tcb *);
extern kernel_ulong_t get_rt_sigframe_addr(struct tcb *);

/**
 * Convert syscall number to syscall name.
 *
 * @param scno Syscall number.
 * @return     String literal corresponding to the syscall number in case latter
 *             is valid; NULL otherwise.
 */
extern const char *syscall_name(kernel_ulong_t scno);
extern const char *err_name(unsigned long err);

extern bool is_erestart(struct tcb *);
extern void temporarily_clear_syserror(struct tcb *);
extern void restore_cleared_syserror(struct tcb *);

extern void *get_tcb_priv_data(const struct tcb *);
extern int set_tcb_priv_data(struct tcb *, void *priv_data,
			     void (*free_priv_data)(void *));
extern void free_tcb_priv_data(struct tcb *);

static inline unsigned long get_tcb_priv_ulong(const struct tcb *tcp)
{
	return (unsigned long) get_tcb_priv_data(tcp);
}

static inline int set_tcb_priv_ulong(struct tcb *tcp, unsigned long val)
{
	return set_tcb_priv_data(tcp, (void *) val, 0);
}

extern int
umoven(struct tcb *tcp, kernel_ulong_t addr, unsigned int len, void *laddr);
#define umove(pid, addr, objp)	\
	umoven((pid), (addr), sizeof(*(objp)), (void *) (objp))

extern int
umoven_or_printaddr(struct tcb *tcp, kernel_ulong_t addr,
		    unsigned int len, void *laddr);
#define umove_or_printaddr(pid, addr, objp)	\
	umoven_or_printaddr((pid), (addr), sizeof(*(objp)), (void *) (objp))

extern int
umoven_or_printaddr_ignore_syserror(struct tcb *tcp, kernel_ulong_t addr,
				    unsigned int len, void *laddr);

extern int
umovestr(struct tcb *tcp, kernel_ulong_t addr, unsigned int len, char *laddr);

extern int upeek(int pid, unsigned long, kernel_ulong_t *);
extern int upoke(int pid, unsigned long, kernel_ulong_t);

extern bool
print_array(struct tcb *tcp,
	    kernel_ulong_t start_addr,
	    size_t nmemb,
	    void *elem_buf,
	    size_t elem_size,
	    int (*umoven_func)(struct tcb *,
				     kernel_ulong_t,
				     unsigned int,
				     void *),
	    bool (*print_func)(struct tcb *,
				     void *elem_buf,
				     size_t elem_size,
				     void *opaque_data),
	    void *opaque_data);

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
extern enum sock_proto getfdproto(struct tcb *, int);

extern const char *xlookup(const struct xlat *, const uint64_t);
extern const char *xlat_search(const struct xlat *, const size_t, const uint64_t);

extern unsigned long get_pagesize(void);
extern int
string_to_uint_ex(const char *str, char **endptr,
		  unsigned int max_val, const char *accepted_ending);
extern int string_to_uint(const char *str);
static inline int
string_to_uint_upto(const char *const str, unsigned int max_val)
{
	return string_to_uint_ex(str, NULL, max_val, NULL);
}
extern int next_set_bit(const void *bit_array, unsigned cur_bit, unsigned size_bits);

#define QUOTE_0_TERMINATED                      0x01
#define QUOTE_OMIT_LEADING_TRAILING_QUOTES      0x02
#define QUOTE_OMIT_TRAILING_0                   0x08
#define QUOTE_FORCE_HEX                         0x10

extern int string_quote(const char *, char *, unsigned int, unsigned int);
extern int print_quoted_string(const char *, unsigned int, unsigned int);

/* a refers to the lower numbered u_arg,
 * b refers to the higher numbered u_arg
 */
#ifdef WORDS_BIGENDIAN
# define ULONG_LONG(a,b) \
	((unsigned long long)(unsigned)(b) | ((unsigned long long)(a)<<32))
#else
# define ULONG_LONG(a,b) \
	((unsigned long long)(unsigned)(a) | ((unsigned long long)(b)<<32))
#endif
extern int getllval(struct tcb *, unsigned long long *, int);
extern int printllval(struct tcb *, const char *, int)
	ATTRIBUTE_FORMAT((printf, 2, 0));

extern void printaddr(kernel_ulong_t addr);
extern int printxvals(const uint64_t, const char *, const struct xlat *, ...)
	ATTRIBUTE_SENTINEL;
extern int printxval_searchn(const struct xlat *xlat, size_t xlat_size,
	uint64_t val, const char *dflt);
#define printxval_search(xlat__, val__, dflt__) \
	printxval_searchn(xlat__, ARRAY_SIZE(xlat__), val__, dflt__)
extern int printargs(struct tcb *);
extern int printargs_u(struct tcb *);
extern int printargs_d(struct tcb *);

extern void addflags(const struct xlat *, uint64_t);
extern int printflags64(const struct xlat *, uint64_t, const char *);
extern const char *sprintflags(const char *, const struct xlat *, uint64_t);
extern const char *sprinttime(long long sec);
extern const char *sprinttime_nsec(long long sec, unsigned long long nsec);
extern const char *sprinttime_usec(long long sec, unsigned long long usec);
extern void print_symbolic_mode_t(unsigned int);
extern void print_numeric_umode_t(unsigned short);
extern void print_numeric_long_umask(unsigned long);
extern void print_dev_t(unsigned long long dev);
extern void print_abnormal_hi(kernel_ulong_t);

extern void
dumpiov_in_msghdr(struct tcb *, kernel_ulong_t addr, kernel_ulong_t data_size);

extern void
dumpiov_in_mmsghdr(struct tcb *, kernel_ulong_t addr);

extern void
dumpiov_upto(struct tcb *, int len, kernel_ulong_t addr, kernel_ulong_t data_size);

extern void
dumpstr(struct tcb *, kernel_ulong_t addr, int len);

extern void
printstr_ex(struct tcb *, kernel_ulong_t addr, kernel_ulong_t len,
	    unsigned int user_style);

extern void
printpathn(struct tcb *, kernel_ulong_t addr, unsigned int n);

extern void
printpath(struct tcb *, kernel_ulong_t addr);

#define TIMESPEC_TEXT_BUFSIZE \
		(sizeof(long long) * 3 * 2 + sizeof("{tv_sec=-, tv_nsec=}"))
extern void printfd(struct tcb *, int);
extern void print_sockaddr(struct tcb *tcp, const void *, int);
extern bool print_sockaddr_by_inode(const unsigned long, const enum sock_proto);
extern bool print_sockaddr_by_inode_cached(const unsigned long);
extern void print_dirfd(struct tcb *, int);

extern int
decode_sockaddr(struct tcb *, kernel_ulong_t addr, int addrlen);

extern void printuid(const char *, const unsigned int);

extern void
print_sigset_addr_len(struct tcb *, kernel_ulong_t addr, kernel_ulong_t len);
extern void
print_sigset_addr(struct tcb *, kernel_ulong_t addr);

extern const char *sprintsigmask_n(const char *, const void *, unsigned int);
#define tprintsigmask_addr(prefix, mask) \
	tprints(sprintsigmask_n((prefix), (mask), sizeof(mask)))
extern void printsignal(int);

extern void
tprint_iov_upto(struct tcb *, kernel_ulong_t len, kernel_ulong_t addr,
		enum iov_decode, kernel_ulong_t data_size);

extern void
decode_netlink(struct tcb *, kernel_ulong_t addr, kernel_ulong_t len);

extern void tprint_open_modes(unsigned int);
extern const char *sprint_open_modes(unsigned int);

extern void
print_seccomp_filter(struct tcb *, kernel_ulong_t addr);

extern void
print_seccomp_fprog(struct tcb *, kernel_ulong_t addr, unsigned short len);

struct strace_stat;
extern void print_struct_stat(struct tcb *tcp, const struct strace_stat *const st);

struct strace_statfs;

extern void
print_struct_statfs(struct tcb *, kernel_ulong_t addr);

extern void
print_struct_statfs64(struct tcb *, kernel_ulong_t addr, kernel_ulong_t size);

extern void print_ifindex(unsigned int);

struct number_set;
extern struct number_set read_set;
extern struct number_set write_set;
extern struct number_set signal_set;

extern bool is_number_in_set(unsigned int number, const struct number_set *);
extern void qualify(const char *);
extern unsigned int qual_flags(const unsigned int);

#define DECL_IOCTL(name)						\
extern int								\
name ## _ioctl(struct tcb *, unsigned int request, kernel_ulong_t arg)
DECL_IOCTL(dm);
DECL_IOCTL(file);
DECL_IOCTL(fs_x);
DECL_IOCTL(nsfs);
DECL_IOCTL(ptp);
DECL_IOCTL(scsi);
DECL_IOCTL(term);
DECL_IOCTL(ubi);
DECL_IOCTL(uffdio);
#undef DECL_IOCTL

extern int decode_sg_io_v4(struct tcb *, const kernel_ulong_t arg);

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

static inline void
printstrn(struct tcb *tcp, kernel_ulong_t addr, kernel_ulong_t len)
{
	printstr_ex(tcp, addr, len, 0);
}

static inline void
printstr(struct tcb *tcp, kernel_ulong_t addr)
{
	printstr_ex(tcp, addr, -1, QUOTE_0_TERMINATED);
}

static inline int
printflags(const struct xlat *x, unsigned int flags, const char *dflt)
{
	return printflags64(x, flags, dflt);
}

static inline int
printxval64(const struct xlat *x, const uint64_t val, const char *dflt)
{
	return printxvals(val, dflt, x, NULL);
}

static inline int
printxval(const struct xlat *x, const unsigned int val, const char *dflt)
{
	return printxvals(val, dflt, x, NULL);
}

static inline void
tprint_iov(struct tcb *tcp, kernel_ulong_t len, kernel_ulong_t addr,
	   enum iov_decode decode_iov)
{
	tprint_iov_upto(tcp, len, addr, decode_iov, -1);
}

#ifdef ALPHA
typedef struct {
	int tv_sec, tv_usec;
} timeval32_t;

extern void print_timeval32_t(const timeval32_t *);
extern void printrusage32(struct tcb *, kernel_ulong_t);
extern const char *sprint_timeval32(struct tcb *tcp, kernel_ulong_t);
extern void print_timeval32(struct tcb *tcp, kernel_ulong_t);
extern void print_timeval32_utimes(struct tcb *tcp, kernel_ulong_t);
extern void print_itimerval32(struct tcb *tcp, kernel_ulong_t);
#endif

#ifdef HAVE_STRUCT_USER_DESC
extern void print_user_desc(struct tcb *, kernel_ulong_t addr);
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
extern void tprintf_comment(const char *fmt, ...) ATTRIBUTE_FORMAT((printf, 1, 2));
extern void tprints_comment(const char *str);

#if SUPPORTED_PERSONALITIES > 1
extern void set_personality(int personality);
extern unsigned current_personality;
#else
# define set_personality(personality) ((void)0)
# define current_personality 0
#endif

#if SUPPORTED_PERSONALITIES == 1
# define current_wordsize PERSONALITY0_WORDSIZE
# define current_klongsize PERSONALITY0_KLONGSIZE
#else
# if SUPPORTED_PERSONALITIES == 2 && PERSONALITY0_WORDSIZE == PERSONALITY1_WORDSIZE
#  define current_wordsize PERSONALITY0_WORDSIZE
# else
extern unsigned current_wordsize;
# endif
# if SUPPORTED_PERSONALITIES == 2 && PERSONALITY0_KLONGSIZE == PERSONALITY1_KLONGSIZE
#  define current_klongsize PERSONALITY0_KLONGSIZE
# else
extern unsigned current_klongsize;
# endif
#endif

#define ANY_WORDSIZE_LESS_THAN_KERNEL_LONG	\
	(SIZEOF_KERNEL_LONG_T > 4		\
	 && (SIZEOF_LONG < SIZEOF_KERNEL_LONG_T || !defined(current_wordsize)))

#define DECL_PRINTNUM(name)						\
extern bool								\
printnum_ ## name(struct tcb *, kernel_ulong_t addr, const char *fmt)	\
	ATTRIBUTE_FORMAT((printf, 3, 0))
DECL_PRINTNUM(short);
DECL_PRINTNUM(int);
DECL_PRINTNUM(int64);
#undef DECL_PRINTNUM

#define DECL_PRINTNUM_ADDR(name)					\
extern bool								\
printnum_addr_ ## name(struct tcb *, kernel_ulong_t addr)
DECL_PRINTNUM_ADDR(int);
DECL_PRINTNUM_ADDR(int64);
#undef DECL_PRINTNUM_ADDR

#ifndef current_wordsize
extern bool
printnum_long_int(struct tcb *, kernel_ulong_t addr,
		  const char *fmt_long, const char *fmt_int)
	ATTRIBUTE_FORMAT((printf, 3, 0))
	ATTRIBUTE_FORMAT((printf, 4, 0));
extern bool printnum_addr_long_int(struct tcb *, kernel_ulong_t addr);
# define printnum_slong(tcp, addr) \
	printnum_long_int((tcp), (addr), "%" PRId64, "%d")
# define printnum_ulong(tcp, addr) \
	printnum_long_int((tcp), (addr), "%" PRIu64, "%u")
# define printnum_ptr(tcp, addr) \
	printnum_addr_long_int((tcp), (addr))
#elif current_wordsize > 4
# define printnum_slong(tcp, addr) \
	printnum_int64((tcp), (addr), "%" PRId64)
# define printnum_ulong(tcp, addr) \
	printnum_int64((tcp), (addr), "%" PRIu64)
# define printnum_ptr(tcp, addr) \
	printnum_addr_int64((tcp), (addr))
#else /* current_wordsize == 4 */
# define printnum_slong(tcp, addr) \
	printnum_int((tcp), (addr), "%d")
# define printnum_ulong(tcp, addr) \
	printnum_int((tcp), (addr), "%u")
# define printnum_ptr(tcp, addr) \
	printnum_addr_int((tcp), (addr))
#endif

#ifndef current_klongsize
extern bool printnum_addr_klong_int(struct tcb *, kernel_ulong_t addr);
# define printnum_kptr(tcp, addr) \
	printnum_addr_klong_int((tcp), (addr))
#elif current_klongsize > 4
# define printnum_kptr(tcp, addr) \
	printnum_addr_int64((tcp), (addr))
#else /* current_klongsize == 4 */
# define printnum_kptr(tcp, addr) \
	printnum_addr_int((tcp), (addr))
#endif

#define DECL_PRINTPAIR(name)						\
extern bool								\
printpair_ ## name(struct tcb *, kernel_ulong_t addr, const char *fmt)	\
	ATTRIBUTE_FORMAT((printf, 3, 0))
DECL_PRINTPAIR(int);
DECL_PRINTPAIR(int64);
#undef DECL_PRINTPAIR

static inline kernel_long_t
truncate_klong_to_current_wordsize(const kernel_long_t v)
{
#if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
	if (current_wordsize < sizeof(v)) {
		return (int) v;
	} else
#endif
	{
		return v;
	}
}

static inline kernel_ulong_t
truncate_kulong_to_current_wordsize(const kernel_ulong_t v)
{
#if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
	if (current_wordsize < sizeof(v)) {
		return (unsigned int) v;
	} else
#endif
	{
		return v;
	}
}

/*
 * Cast a pointer or a pointer-sized integer to kernel_ulong_t.
 */
#define ptr_to_kulong(v) ((kernel_ulong_t) (unsigned long) (v))

/*
 * Zero-extend a signed integer type to unsigned long long.
 */
#define zero_extend_signed_to_ull(v) \
	(sizeof(v) == sizeof(char) ? (unsigned long long) (unsigned char) (v) : \
	 sizeof(v) == sizeof(short) ? (unsigned long long) (unsigned short) (v) : \
	 sizeof(v) == sizeof(int) ? (unsigned long long) (unsigned int) (v) : \
	 sizeof(v) == sizeof(long) ? (unsigned long long) (unsigned long) (v) : \
	 (unsigned long long) (v))

/*
 * Sign-extend an unsigned integer type to long long.
 */
#define sign_extend_unsigned_to_ll(v) \
	(sizeof(v) == sizeof(char) ? (long long) (char) (v) : \
	 sizeof(v) == sizeof(short) ? (long long) (short) (v) : \
	 sizeof(v) == sizeof(int) ? (long long) (int) (v) : \
	 sizeof(v) == sizeof(long) ? (long long) (long) (v) : \
	 (long long) (v))

extern const struct_sysent sysent0[];
extern const char *const errnoent0[];
extern const char *const signalent0[];
extern const struct_ioctlent ioctlent0[];

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

extern const unsigned int nsyscall_vec[SUPPORTED_PERSONALITIES];
extern const struct_sysent *const sysent_vec[SUPPORTED_PERSONALITIES];
extern struct inject_opts *inject_vec[SUPPORTED_PERSONALITIES];

#ifdef IN_MPERS_BOOTSTRAP
/* Transform multi-line MPERS_PRINTER_DECL statements to one-liners.  */
# define MPERS_PRINTER_DECL(type, name, ...) MPERS_PRINTER_DECL(type, name, __VA_ARGS__)
#else /* !IN_MPERS_BOOTSTRAP */
# if SUPPORTED_PERSONALITIES > 1
#  include "printers.h"
# else
#  include "native_printer_decls.h"
# endif
# define MPERS_PRINTER_DECL(type, name, ...) type MPERS_FUNC_NAME(name)(__VA_ARGS__)
#endif /* !IN_MPERS_BOOTSTRAP */

/* Checks that sysent[scno] is not out of range. */
static inline bool
scno_in_range(kernel_ulong_t scno)
{
	return scno < nsyscalls;
}

/*
 * Checks whether scno is not out of range,
 * its corresponding sysent[scno].sys_func is non-NULL,
 * and its sysent[scno].sys_flags has no TRACE_INDIRECT_SUBCALL flag set.
 */
static inline bool
scno_is_valid(kernel_ulong_t scno)
{
	return scno_in_range(scno)
	       && sysent[scno].sys_func
	       && !(sysent[scno].sys_flags & TRACE_INDIRECT_SUBCALL);
}

#define MPERS_FUNC_NAME__(prefix, name) prefix ## name
#define MPERS_FUNC_NAME_(prefix, name) MPERS_FUNC_NAME__(prefix, name)
#define MPERS_FUNC_NAME(name) MPERS_FUNC_NAME_(MPERS_PREFIX, name)

#define SYS_FUNC_NAME(syscall_name) MPERS_FUNC_NAME(syscall_name)

#define SYS_FUNC(syscall_name) int SYS_FUNC_NAME(sys_ ## syscall_name)(struct tcb *tcp)

#if SIZEOF_KERNEL_LONG_T > SIZEOF_LONG
# define PRI_kl "ll"
#else
# define PRI_kl "l"
#endif

#define PRI_kld PRI_kl"d"
#define PRI_klu PRI_kl"u"
#define PRI_klx PRI_kl"x"

/*
 * The kernel used to define 64-bit types on 64-bit systems on a per-arch
 * basis.  Some architectures would use unsigned long and others would use
 * unsigned long long.  These types were exported as part of the
 * kernel-userspace ABI and now must be maintained forever.  This matches
 * what the kernel exports for each architecture so we don't need to cast
 * every printing of __u64 or __s64 to stdint types.
 */
#if SIZEOF_LONG == 4
# define PRI__64 "ll"
#elif defined ALPHA || defined IA64 || defined MIPS || defined POWERPC
# define PRI__64 "l"
#else
# define PRI__64 "ll"
#endif

#define PRI__d64 PRI__64"d"
#define PRI__u64 PRI__64"u"
#define PRI__x64 PRI__64"x"

#endif /* !STRACE_DEFS_H */
