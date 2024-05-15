/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 1999-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "nsig.h"
#include "xstring.h"

/* The libc headers do not define this constant since it should only be
   used by the implementation.  So we define it here.  */
#ifndef SA_RESTORER
# ifdef ASM_SA_RESTORER
#  define SA_RESTORER ASM_SA_RESTORER
# endif
#endif

/*
 * Some architectures define SA_RESTORER in their headers,
 * but do not actually have sa_restorer.
 *
 * Some architectures, otherwise, do not define SA_RESTORER in their headers,
 * but actually have sa_restorer.
 */
#ifdef HAVE_ARCH_SA_RESTORER
# define HAVE_SA_RESTORER HAVE_ARCH_SA_RESTORER
#else /* !HAVE_ARCH_SA_RESTORER */
# ifdef SA_RESTORER
#  define HAVE_SA_RESTORER 1
# else
#  define HAVE_SA_RESTORER 0
# endif
#endif /* HAVE_ARCH_SA_RESTORER */

#include "xlat/sa_handler_values.h"
#include "xlat/sigact_flags.h"
#include "xlat/sigprocmaskcmds.h"

/* Anonymous realtime signals. */
#ifndef ASM_SIGRTMIN
/* Linux kernel >= 3.18 defines SIGRTMIN to 32 on all architectures. */
# define ASM_SIGRTMIN 32
#endif
#ifndef ASM_SIGRTMAX
/* Under glibc 2.1, SIGRTMAX et al are functions, but __SIGRTMAX is a
   constant.  This is what we want.  Otherwise, just use SIGRTMAX. */
# ifdef SIGRTMAX
#  ifndef __SIGRTMAX
#   define __SIGRTMAX SIGRTMAX
#  endif
# endif
# ifdef __SIGRTMAX
#  define ASM_SIGRTMAX __SIGRTMAX
# endif
#endif

/* Note on the size of sigset_t:
 *
 * In glibc, sigset_t is an array with space for 1024 bits (!),
 * even though all arches supported by Linux have only 64 signals
 * except MIPS, which has 128. IOW, it is 128 bytes long.
 *
 * In-kernel sigset_t is sized correctly (it is either 64 or 128 bit long).
 * However, some old syscall return only 32 lower bits (one word).
 * Example: sys_sigpending vs sys_rt_sigpending.
 *
 * Be aware of this fact when you try to
 *     memcpy(&tcp->u_arg[1], &something, sizeof(sigset_t))
 * - sizeof(sigset_t) is much bigger than you think,
 * it may overflow tcp->u_arg[] array, and it may try to copy more data
 * than is really available in <something>.
 * Similarly,
 *     umoven(tcp, addr, sizeof(sigset_t), &sigset)
 * may be a bad idea: it'll try to read much more data than needed
 * to fetch a sigset_t.
 * Use NSIG_BYTES as a size instead.
 */

static const char *
get_sa_handler_str(kernel_ulong_t handler)
{
	return xlookup(sa_handler_values, handler);
}

static void
print_sa_handler(kernel_ulong_t handler)
{
	const char *sa_handler_str = get_sa_handler_str(handler);

	if (sa_handler_str)
		print_xlat_ex(handler, sa_handler_str, XLAT_STYLE_DEFAULT);
	else
		printaddr(handler);
}

const char *
signame(const int sig)
{
	if (sig > 0) {
		const unsigned int s = sig;

		if (s < nsignals)
			return signalent[s];
#ifdef ASM_SIGRTMAX
		if (s >= ASM_SIGRTMIN && s <= (unsigned int) ASM_SIGRTMAX) {
			static char buf[sizeof("SIGRT_%u") + sizeof(s) * 3];

			xsprintf(buf, "SIGRT_%u", s - ASM_SIGRTMIN);
			return buf;
		}
#endif
	}

	return NULL;
}

const char *
sprintsigname(const int sig)
{
	const char *str = signame(sig);

	if (str)
		return str;

	static char buf[sizeof(sig) * 3 + 2];

	xsprintf(buf, "%d", sig);

	return buf;
}

const char *
sprintsigmask_n(const char *prefix, const void *sig_mask, unsigned int bytes)
{
	/*
	 * The maximum number of signal names to be printed
	 * is NSIG_BYTES * 8 * 2 / 3.
	 * Most of signal names have length 7,
	 * average length of signal names is less than 7.
	 * The length of prefix string does not exceed 16.
	 */
	static char outstr[128 + 8 * (NSIG_BYTES * 8 * 2 / 3)];

	char *s;
	const uint32_t *mask;
	uint32_t inverted_mask[NSIG_BYTES / 4];
	unsigned int size;
	char sep;

	s = stpcpy(outstr, prefix);

	mask = sig_mask;
	/* length of signal mask in 4-byte words */
	size = ROUNDUP_DIV(MIN(bytes, NSIG_BYTES), 4);

	/* check whether 2/3 or more bits are set */
	if (popcount32(mask, size) >= size * (4 * 8) * 2 / 3) {
		/* show those signals that are NOT in the mask */
		for (unsigned int j = 0; j < size; ++j)
			inverted_mask[j] = ~mask[j];
		mask = inverted_mask;
		*s++ = '~';
	}

	sep = '[';
	for (int i = 0; (i = next_set_bit(mask, i, size * (4 * 8))) >= 0; ) {
		++i;
		*s++ = sep;
		if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
			s = xappendstr(outstr, s, "%u", i);
		if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
			s = xappendstr(outstr, s, " /* ");
		if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW) {
			if ((unsigned) i < nsignals) {
				s = stpcpy(s, signalent[i] + 3);
			}
#ifdef ASM_SIGRTMAX
			else if (i >= ASM_SIGRTMIN && i <= ASM_SIGRTMAX) {
				s = xappendstr(outstr, s, "RT_%u",
					       i - ASM_SIGRTMIN);
			}
#endif
			else if (xlat_verbose(xlat_verbosity)
				 != XLAT_STYLE_ABBREV) {
				s = xappendstr(outstr, s, "%u", i);
			}
		}
		if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
			s = xappendstr(outstr, s, " */");
		sep = ' ';
	}
	if (sep == '[')
		*s++ = sep;
	*s++ = ']';
	*s = '\0';
	return outstr;
}

#define sprintsigmask_val(prefix, mask) \
	sprintsigmask_n((prefix), &(mask), sizeof(mask))

#define tprintsigmask_val(mask) \
	tprints_string(sprintsigmask_n("", &(mask), sizeof(mask)))

static const char *
sprint_old_sigmask_val(const char *const prefix, const unsigned long mask)
{
#if defined(current_wordsize) || !defined(WORDS_BIGENDIAN)
	return sprintsigmask_n(prefix, &mask, current_wordsize);
#else /* !current_wordsize && WORDS_BIGENDIAN */
	if (current_wordsize == sizeof(mask)) {
		return sprintsigmask_val(prefix, mask);
	} else {
		uint32_t mask32 = mask;
		return sprintsigmask_val(prefix, mask32);
	}
#endif
}

static void
tprint_old_sigmask_val(const unsigned long mask)
{
	tprints_string(sprint_old_sigmask_val("", mask));
}

void
printsignal(int nr)
{
	const char *str = signame(nr);

	if (!str || xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		PRINT_VAL_D(nr);
	if (!str || xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;
	(xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE
		? tprints_comment : tprints_string)(str);
}

static void
print_sigset_addr_len_limit(struct tcb *const tcp, const kernel_ulong_t addr,
			    const kernel_ulong_t len, const unsigned int min_len)
{
	/*
	 * Here len is usually equal to NSIG_BYTES or current_wordsize.
	 * But we code this defensively:
	 */
	if (len < min_len || len > NSIG_BYTES) {
		printaddr(addr);
		return;
	}
	int mask[NSIG_BYTES / sizeof(int)] = {};
	if (umoven_or_printaddr(tcp, addr, len, mask))
		return;
	tprints_string(sprintsigmask_n("", mask, len));
}

void
print_sigset_addr_len(struct tcb *const tcp, const kernel_ulong_t addr,
		      const kernel_ulong_t len)
{
	print_sigset_addr_len_limit(tcp, addr, len, current_wordsize);
}

void
print_sigset_addr(struct tcb *const tcp, const kernel_ulong_t addr)
{
	tprint_struct_begin();
	tprints_field_name("mask");
	print_sigset_addr_len_limit(tcp, addr, NSIG_BYTES, NSIG_BYTES);
	tprint_struct_end();
}

SYS_FUNC(ssetmask)
{
	if (entering(tcp)) {
		tprint_old_sigmask_val((unsigned) tcp->u_arg[0]);
	} else if (!syserror(tcp)) {
		tcp->auxstr = sprint_old_sigmask_val("old mask ",
						     (unsigned) tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	}
	return 0;
}

struct old_sigaction {
	/* sa_handler may be a libc #define, need to use other name: */
#if defined MIPS
	unsigned int sa_flags;
	unsigned long sa_handler__;
	unsigned long sa_mask;
#elif defined ALPHA
	unsigned long sa_handler__;
	unsigned long sa_mask;
	unsigned int sa_flags;
#else
	unsigned long sa_handler__;
	unsigned long sa_mask;
	unsigned long sa_flags;
	unsigned long sa_restorer;
#endif
}
#ifdef ALPHA
	ATTRIBUTE_PACKED
#endif
;

static void
decode_old_sigaction(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct old_sigaction sa;

#ifndef current_wordsize
	if (current_wordsize < sizeof(sa.sa_handler__)) {
		struct old_sigaction32 {
			uint32_t sa_handler__;
			uint32_t sa_mask;
			uint32_t sa_flags;
			uint32_t sa_restorer;
		} sa32;

		if (umove_or_printaddr(tcp, addr, &sa32))
			return;

		memset(&sa, 0, sizeof(sa));
		sa.sa_handler__ = sa32.sa_handler__;
		sa.sa_flags = sa32.sa_flags;
		sa.sa_restorer = sa32.sa_restorer;
		sa.sa_mask = sa32.sa_mask;
	} else
#endif
	if (umove_or_printaddr(tcp, addr, &sa))
		return;

	tprint_struct_begin();
	tprints_field_name("sa_handler");
	print_sa_handler(sa.sa_handler__);
	tprint_struct_next();
	PRINT_FIELD_OBJ_VAL(sa, sa_mask, tprint_old_sigmask_val);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(sa, sa_flags, sigact_flags, "SA_???");
#if !(defined ALPHA || defined MIPS)
	if (sa.sa_flags & 0x04000000U) {
		tprint_struct_next();
		PRINT_FIELD_OBJ_VAL(sa, sa_restorer, printaddr);
	}
#endif
	tprint_struct_end();
}

SYS_FUNC(sigaction)
{
	if (entering(tcp)) {
		int signo = tcp->u_arg[0];
#if defined SPARC || defined SPARC64
		if (signo < 0) {
			tprints_string("-");
			signo = -signo;
		}
#endif
		/* signum */
		printsignal(signo);
		tprint_arg_next();

		/* act */
		decode_old_sigaction(tcp, tcp->u_arg[1]);
		tprint_arg_next();
	} else {
		/* oldact */
		decode_old_sigaction(tcp, tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(signal)
{
	if (entering(tcp)) {
		/* signum */
		printsignal(tcp->u_arg[0]);
		tprint_arg_next();

		/* handler */
		print_sa_handler(tcp->u_arg[1]);
		return 0;
	} else if (!syserror(tcp)) {
		tcp->auxstr = get_sa_handler_str(tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	}
	return 0;
}

SYS_FUNC(sgetmask)
{
	if (exiting(tcp) && !syserror(tcp)) {
		tcp->auxstr = sprint_old_sigmask_val("mask ", tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	}
	return 0;
}

SYS_FUNC(sigsuspend)
{
	/* mask */
#ifdef MIPS
	print_sigset_addr_len(tcp, tcp->u_arg[n_args(tcp) - 1],
			      current_wordsize);
#else
	tprint_old_sigmask_val(tcp->u_arg[n_args(tcp) - 1]);
#endif

	return RVAL_DECODED;
}

#ifdef ALPHA
/*
 * The OSF/1 sigprocmask is different: it doesn't pass in two pointers,
 * but rather passes in the new bitmask as an argument and then returns
 * the old bitmask.  This "works" because we only have 64 signals to worry
 * about.  If you want more, use of the rt_sigprocmask syscall is required.
 *
 * Alpha:
 *	old = osf_sigprocmask(how, new);
 * Everyone else:
 *	ret = sigprocmask(how, &new, &old, ...);
 */
SYS_FUNC(osf_sigprocmask)
{
	if (entering(tcp)) {
		/* how */
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprint_arg_next();

		/* set */
		tprintsigmask_val(tcp->u_arg[1]);
	} else if (!syserror(tcp)) {
		tcp->auxstr = sprintsigmask_val("old mask ", tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	}
	return 0;
}

#else /* !ALPHA */

/* "Old" sigprocmask, which operates with word-sized signal masks */
SYS_FUNC(sigprocmask)
{
	if (entering(tcp)) {
		/* how */
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprint_arg_next();

		/* set */
		print_sigset_addr_len(tcp, tcp->u_arg[1], current_wordsize);
		tprint_arg_next();
	} else {
		/* oldset */
		print_sigset_addr_len(tcp, tcp->u_arg[2], current_wordsize);
	}
	return 0;
}
#endif /* !ALPHA */

SYS_FUNC(kill)
{
	/* pid */
	printpid_tgid_pgid(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* signum */
	printsignal(tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(tkill)
{
	/* tid */
	printpid(tcp, tcp->u_arg[0], PT_TID);
	tprint_arg_next();

	/* signum */
	printsignal(tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(tgkill)
{
	/* tgid */
	printpid(tcp, tcp->u_arg[0], PT_TGID);
	tprint_arg_next();

	/* tid */
	printpid(tcp, tcp->u_arg[1], PT_TID);
	tprint_arg_next();

	/* signum */
	printsignal(tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(sigpending)
{
	if (exiting(tcp)) {
		/* set */
		print_sigset_addr_len(tcp, tcp->u_arg[0], current_wordsize);
	}
	return 0;
}

SYS_FUNC(rt_sigprocmask)
{
	/* Note: arg[3] is the length of the sigset. Kernel requires NSIG_BYTES */
	if (entering(tcp)) {
		/* how */
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprint_arg_next();

		/* set */
		print_sigset_addr_len(tcp, tcp->u_arg[1], tcp->u_arg[3]);
		tprint_arg_next();
	} else {
		/* oldset */
		print_sigset_addr_len(tcp, tcp->u_arg[2], tcp->u_arg[3]);
		tprint_arg_next();

		/* sigsetsize */
		PRINT_VAL_U(tcp->u_arg[3]);
	}
	return 0;
}

/* Structure describing the action to be taken when a signal arrives.  */
struct new_sigaction {
	/* sa_handler may be a libc #define, need to use other name: */
#ifdef MIPS
	unsigned int sa_flags;
	unsigned long sa_handler__;
#else
	unsigned long sa_handler__;
	unsigned long sa_flags;
#endif /* !MIPS */
#if HAVE_SA_RESTORER
	unsigned long sa_restorer;
#endif
	/* Kernel treats sa_mask as an array of longs. */
	unsigned long sa_mask[NSIG / sizeof(long)];
};
/* Same for i386-on-x86_64 and similar cases */
struct new_sigaction32 {
	uint32_t sa_handler__;
	uint32_t sa_flags;
#if HAVE_SA_RESTORER
	uint32_t sa_restorer;
#endif
	uint32_t sa_mask[2 * (NSIG / sizeof(long))];
};

static void
decode_new_sigaction(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct new_sigaction sa;

#ifndef current_wordsize
	if (current_wordsize < sizeof(sa.sa_handler__)) {
		struct new_sigaction32 sa32;

		if (umove_or_printaddr(tcp, addr, &sa32))
			return;

		memset(&sa, 0, sizeof(sa));
		sa.sa_handler__ = sa32.sa_handler__;
		sa.sa_flags     = sa32.sa_flags;
# if HAVE_SA_RESTORER && defined SA_RESTORER
		sa.sa_restorer  = sa32.sa_restorer;
# endif
		/* Kernel treats sa_mask as an array of longs.
		 * For 32-bit process, "long" is uint32_t, thus, for example,
		 * 32th bit in sa_mask will end up as bit 0 in sa_mask[1].
		 * But for (64-bit) kernel, 32th bit in sa_mask is
		 * 32th bit in 0th (64-bit) long!
		 * For little-endian, it's the same.
		 * For big-endian, we swap 32-bit words.
		 */
		sa.sa_mask[0] = ULONG_LONG(sa32.sa_mask[0], sa32.sa_mask[1]);
	} else
#endif
	if (umove_or_printaddr(tcp, addr, &sa))
		return;

	tprint_struct_begin();
	tprints_field_name("sa_handler");
	print_sa_handler(sa.sa_handler__);
	/*
	 * Sigset size is in tcp->u_arg[4] (SPARC)
	 * or in tcp->u_arg[3] (all other),
	 * but kernel won't handle sys_rt_sigaction
	 * with wrong sigset size (just returns EINVAL instead).
	 * We just fetch the right size, which is NSIG_BYTES.
	 */
	tprint_struct_next();
	tprints_field_name("sa_mask");
	tprintsigmask_val(sa.sa_mask);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(sa, sa_flags, sigact_flags, "SA_???");
#if HAVE_SA_RESTORER && defined SA_RESTORER
	if (sa.sa_flags & SA_RESTORER) {
		tprint_struct_next();
		PRINT_FIELD_OBJ_VAL(sa, sa_restorer, printaddr);
	}
#endif
	tprint_struct_end();
}

SYS_FUNC(rt_sigaction)
{
	if (entering(tcp)) {
		/* signum */
		printsignal(tcp->u_arg[0]);
		tprint_arg_next();

		/* act */
		decode_new_sigaction(tcp, tcp->u_arg[1]);
		tprint_arg_next();
	} else {
		/* oldact */
		decode_new_sigaction(tcp, tcp->u_arg[2]);
		tprint_arg_next();

#if defined(SPARC) || defined(SPARC64)
		/* sa_restorer */
		PRINT_VAL_X(tcp->u_arg[3]);
		tprint_arg_next();

		/* sigsetsize */
		PRINT_VAL_U(tcp->u_arg[4]);
#elif defined(ALPHA)
		/* sigsetsize */
		PRINT_VAL_U(tcp->u_arg[3]);
		tprint_arg_next();

		/* sa_restorer */
		PRINT_VAL_X(tcp->u_arg[4]);
#else
		/* sigsetsize */
		PRINT_VAL_U(tcp->u_arg[3]);
#endif
	}
	return 0;
}

SYS_FUNC(rt_sigpending)
{
	if (exiting(tcp)) {
		/*
		 * One of the few syscalls where sigset size (arg[1])
		 * is allowed to be <= NSIG_BYTES, not strictly ==.
		 * This allows non-rt sigpending() syscall
		 * to reuse rt_sigpending() code in kernel.
		 */
		print_sigset_addr_len_limit(tcp, tcp->u_arg[0],
					    tcp->u_arg[1], 1);
		tprint_arg_next();

		/* sigsetsize */
		PRINT_VAL_U(tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(rt_sigsuspend)
{
	/* NB: kernel requires arg[1] == NSIG_BYTES */
	print_sigset_addr_len(tcp, tcp->u_arg[0], tcp->u_arg[1]);
	tprint_arg_next();

	/* sigsetsize */
	PRINT_VAL_U(tcp->u_arg[1]);

	return RVAL_DECODED;
}

static void
print_sigqueueinfo(struct tcb *const tcp, const int sig,
		   const kernel_ulong_t addr)
{
	/* signum */
	printsignal(sig);
	tprint_arg_next();

	/* info */
	printsiginfo_at(tcp, addr);
}

SYS_FUNC(rt_sigqueueinfo)
{
	/* tgid */
	printpid(tcp, tcp->u_arg[0], PT_TGID);
	tprint_arg_next();

	/* int sig, siginfo_t *info */
	print_sigqueueinfo(tcp, tcp->u_arg[1], tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(rt_tgsigqueueinfo)
{
	/* tgid */
	printpid(tcp, tcp->u_arg[0], PT_TGID);
	tprint_arg_next();

	/* tid */
	printpid(tcp, tcp->u_arg[1], PT_TID);
	tprint_arg_next();

	/* int sig, siginfo_t *info */
	print_sigqueueinfo(tcp, tcp->u_arg[2], tcp->u_arg[3]);

	return RVAL_DECODED;
}

#include "xlat/pidfd_send_signal_flags.h"

SYS_FUNC(pidfd_send_signal)
{
	/* int pidfd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* int sig, siginfo_t *info */
	print_sigqueueinfo(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprint_arg_next();

	/* unsigned int flags */
	printflags(pidfd_send_signal_flags, tcp->u_arg[3], "PIDFD_SIGNAL_???");

	return RVAL_DECODED;
}

static int
do_rt_sigtimedwait(struct tcb *const tcp, const print_obj_by_addr_fn print_ts,
		   const sprint_obj_by_addr_fn sprint_ts)
{
	/* NB: kernel requires arg[3] == NSIG_BYTES */
	if (entering(tcp)) {
		/* set */
		print_sigset_addr_len(tcp, tcp->u_arg[0], tcp->u_arg[3]);
		tprint_arg_next();

		if (!(tcp->u_arg[1] && verbose(tcp))) {
			/*
			 * This is the only "return" parameter,
			 * if we are not going to fetch it on exit,
			 * decode all parameters on entry.
			 */
			printaddr(tcp->u_arg[1]);
			tprint_arg_next();

			/* timeout */
			print_ts(tcp, tcp->u_arg[2]);
			tprint_arg_next();

			/* sigsetsize */
			PRINT_VAL_U(tcp->u_arg[3]);
		} else {
			char *sts = xstrdup(sprint_ts(tcp, tcp->u_arg[2]));
			set_tcb_priv_data(tcp, sts, free);
		}
	} else {
		if (tcp->u_arg[1] && verbose(tcp)) {
			/* info */
			printsiginfo_at(tcp, tcp->u_arg[1]);
			tprint_arg_next();

			/* timeout */
			tprints_string(get_tcb_priv_data(tcp));
			tprint_arg_next();

			/* sigsetsize */
			PRINT_VAL_U(tcp->u_arg[3]);
		}

		if (!syserror(tcp) && tcp->u_rval) {
			tcp->auxstr = signame(tcp->u_rval);
			return RVAL_STR;
		}
	}
	return 0;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(rt_sigtimedwait_time32)
{
	return do_rt_sigtimedwait(tcp, print_timespec32, sprint_timespec32);
}
#endif

SYS_FUNC(rt_sigtimedwait_time64)
{
	return do_rt_sigtimedwait(tcp, print_timespec64, sprint_timespec64);
}

SYS_FUNC(restart_syscall)
{
	tprintf_string("<... resuming interrupted %s ...>",
		tcp->s_prev_ent ? tcp->s_prev_ent->sys_name : "system call");

	return RVAL_DECODED;
}
