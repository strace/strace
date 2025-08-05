/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_fcntl.h"
#include <signal.h>
#include <sys/timex.h>

#if HAVE_ARCH_TIME32_SYSCALLS || HAVE_ARCH_OLD_TIME64_SYSCALLS
static void
print_timezone(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct timezone tz;

	if (umove_or_printaddr(tcp, addr, &tz))
		return;

	tprint_struct_begin();
	PRINT_FIELD_D(tz, tz_minuteswest);
	tprint_struct_next();
	PRINT_FIELD_D(tz, tz_dsttime);
	tprint_struct_end();
}

SYS_FUNC(gettimeofday)
{
	if (exiting(tcp)) {
		/* tv */
		tprints_arg_name("tv");
		print_timeval(tcp, tcp->u_arg[0]);

		/* tz */
		tprints_arg_next_name("tz");
		print_timezone(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(settimeofday)
{
	/* tv */
	tprints_arg_name("tv");
	print_timeval(tcp, tcp->u_arg[0]);

	/* tz */
	tprints_arg_next_name("tz");
	print_timezone(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}
#endif

#ifdef ALPHA
SYS_FUNC(osf_gettimeofday)
{
	if (exiting(tcp)) {
		/* tv */
		tprints_arg_name("tv");
		print_timeval32(tcp, tcp->u_arg[0]);

		/* tz */
		tprints_arg_next_name("tz");
		print_timezone(tcp, tcp->u_arg[1]);
	}
	return 0;
}
#endif

#ifdef ALPHA
SYS_FUNC(osf_settimeofday)
{
	/* tv */
	tprints_arg_name("tv");
	print_timeval32(tcp, tcp->u_arg[0]);

	/* tz */
	tprints_arg_next_name("tz");
	print_timezone(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}
#endif

#if HAVE_ARCH_TIME32_SYSCALLS || HAVE_ARCH_OLD_TIME64_SYSCALLS
static int
do_nanosleep(struct tcb *const tcp, const print_obj_by_addr_fn print_ts)
{
	if (entering(tcp)) {
		/* req */
		tprints_arg_name("req");
		print_ts(tcp, tcp->u_arg[0]);
	} else {
		/* rem */
		/*
		 * Second (returned) timespec is only significant if syscall
		 * was interrupted.  On success and in case of other errors we
		 * print only its address, since kernel doesn't modify it,
		 * and printing the value may show uninitialized data.
		 */
		tprints_arg_next_name("rem");
		if (is_erestart(tcp)) {
			temporarily_clear_syserror(tcp);
			print_ts(tcp, tcp->u_arg[1]);
			restore_cleared_syserror(tcp);
		} else {
			printaddr(tcp->u_arg[1]);
		}
	}
	return 0;
}
#endif /* HAVE_ARCH_TIME32_SYSCALLS || HAVE_ARCH_OLD_TIME64_SYSCALLS */

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(nanosleep_time32)
{
	return do_nanosleep(tcp, print_timespec32);
}
#endif

#if HAVE_ARCH_OLD_TIME64_SYSCALLS
SYS_FUNC(nanosleep_time64)
{
	return do_nanosleep(tcp, print_timespec64);
}
#endif

#include "xlat/itimer_which.h"

SYS_FUNC(getitimer)
{
	if (entering(tcp)) {
		/* which */
		tprints_arg_name("which");
		printxval(itimer_which, tcp->u_arg[0], "ITIMER_???");
	} else {
		/* curr_value */
		tprints_arg_next_name("curr_value");
		print_itimerval(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef ALPHA
SYS_FUNC(osf_getitimer)
{
	if (entering(tcp)) {
		/* which */
		tprints_arg_name("which");
		printxval(itimer_which, tcp->u_arg[0], "ITIMER_???");
	} else {
		/* curr_value */
		tprints_arg_next_name("curr_value");
		print_itimerval32(tcp, tcp->u_arg[1]);
	}
	return 0;
}
#endif

SYS_FUNC(setitimer)
{
	if (entering(tcp)) {
		/* which */
		tprints_arg_name("which");
		printxval(itimer_which, tcp->u_arg[0], "ITIMER_???");

		/* new_value */
		tprints_arg_next_name("new_value");
		print_itimerval(tcp, tcp->u_arg[1]);
	} else {
		/* old_value */
		tprints_arg_next_name("old_value");
		print_itimerval(tcp, tcp->u_arg[2]);
	}
	return 0;
}

#ifdef ALPHA
SYS_FUNC(osf_setitimer)
{
	if (entering(tcp)) {
		/* which */
		tprints_arg_name("which");
		printxval(itimer_which, tcp->u_arg[0], "ITIMER_???");

		/* new_value */
		tprints_arg_next_name("new_value");
		print_itimerval32(tcp, tcp->u_arg[1]);
	} else {
		/* old_value */
		tprints_arg_next_name("old_value");
		print_itimerval32(tcp, tcp->u_arg[2]);
	}
	return 0;
}
#endif

#include "xlat/adjtimex_state.h"

static int
do_adjtimex(struct tcb *const tcp, const print_obj_by_addr_fn print_tx,
	    const kernel_ulong_t addr)
{
	/* buf */
	tprints_arg_name("buf");
	if (print_tx(tcp, addr))
		return 0;
	tcp->auxstr = xlookup(adjtimex_state, (kernel_ulong_t) tcp->u_rval);
	return RVAL_STR;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(adjtimex32)
{
	if (exiting(tcp))
		return do_adjtimex(tcp, print_timex32, tcp->u_arg[0]);
	return 0;
}
#endif

#if HAVE_ARCH_OLD_TIME64_SYSCALLS
SYS_FUNC(adjtimex64)
{
	if (exiting(tcp))
# ifndef SPARC64
		return do_adjtimex(tcp, print_timex64, tcp->u_arg[0]);
# else
		return do_adjtimex(tcp, print_sparc64_timex, tcp->u_arg[0]);
# endif
	return 0;
}
#endif

#include "xlat/clockflags.h"
#include "xlat/clocknames.h"

static void
printclockname(int clockid)
{
#ifdef CLOCKID_TO_FD
# include "xlat/cpuclocknames.h"

	if (clockid < 0) {
		if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
			PRINT_VAL_D(clockid);

		if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
			return;

		if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
			tprint_comment_begin();

		if ((clockid & CLOCKFD_MASK) == CLOCKFD)
			tprints_arg_begin("FD_TO_CLOCKID");
			PRINT_VAL_D(CLOCKID_TO_FD(clockid));
		else {
			tprints_arg_begin(CPUCLOCK_PERTHREAD(clockid) ?
					  "MAKE_THREAD_CPUCLOCK" :
					  "MAKE_PROCESS_CPUCLOCK");
			PRINT_VAL_D(CPUCLOCK_PID(clockid));
			tprint_arg_next();
			printxval(cpuclocknames, clockid & CLOCKFD_MASK,
				  "CPUCLOCK_???");
		}
		tprint_arg_end();

		if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
			tprint_comment_end();
	} else
#endif
		printxval(clocknames, clockid, "CLOCK_???");
}

static int
do_clock_settime(struct tcb *const tcp, const print_obj_by_addr_fn print_ts)
{
	/* clockid */
	tprints_arg_name("clockid");
	printclockname(tcp->u_arg[0]);

	/* tp */
	tprints_arg_next_name("tp");
	print_ts(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(clock_settime32)
{
	return do_clock_settime(tcp, print_timespec32);
}
#endif

SYS_FUNC(clock_settime64)
{
	return do_clock_settime(tcp, print_timespec64);
}

static int
do_clock_gettime(struct tcb *const tcp, const print_obj_by_addr_fn print_ts)
{
	if (entering(tcp)) {
		/* clockid */
		tprints_arg_name("clockid");
		printclockname(tcp->u_arg[0]);
	} else {
		/* tp */
		tprints_arg_next_name("tp");
		print_ts(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(clock_gettime32)
{
	return do_clock_gettime(tcp, print_timespec32);
}
#endif

SYS_FUNC(clock_gettime64)
{
	return do_clock_gettime(tcp, print_timespec64);
}

static int
do_clock_nanosleep(struct tcb *const tcp, const print_obj_by_addr_fn print_ts)
{
	if (entering(tcp)) {
		/* clockid */
		tprints_arg_name("clockid");
		printclockname(tcp->u_arg[0]);

		/* flags */
		tprints_arg_next_name("flags");
		printflags(clockflags, tcp->u_arg[1], "TIMER_???");

		/* request */
		tprints_arg_next_name("request");
		print_ts(tcp, tcp->u_arg[2]);
	} else {
		/* remain */
		/*
		 * Second (returned) timespec is only significant
		 * if syscall was interrupted and flags is not TIMER_ABSTIME.
		 */
		tprints_arg_next_name("remain");
		if (!tcp->u_arg[1] && is_erestart(tcp)) {
			temporarily_clear_syserror(tcp);
			print_ts(tcp, tcp->u_arg[3]);
			restore_cleared_syserror(tcp);
		} else {
			printaddr(tcp->u_arg[3]);
		}
	}
	return 0;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(clock_nanosleep_time32)
{
	return do_clock_nanosleep(tcp, print_timespec32);
}
#endif

SYS_FUNC(clock_nanosleep_time64)
{
	return do_clock_nanosleep(tcp, print_timespec64);
}

static int
do_clock_adjtime(struct tcb *const tcp, const print_obj_by_addr_fn print_tx)
{
	if (entering(tcp)) {
		/* clockid */
		tprints_arg_name("clockid");
		printclockname(tcp->u_arg[0]);
		return 0;
	} else {
		tprint_arg_next();
		return do_adjtimex(tcp, print_tx, tcp->u_arg[1]);
	}
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(clock_adjtime32)
{
	return do_clock_adjtime(tcp, print_timex32);
}
#endif

SYS_FUNC(clock_adjtime64)
{
	return do_clock_adjtime(tcp, print_timex64);
}

#ifdef SPARC64
SYS_FUNC(clock_sparc64_adjtime)
{
	return do_clock_adjtime(tcp, print_sparc64_timex);
}
#endif

SYS_FUNC(timer_create)
{
	if (entering(tcp)) {
		/* clockid */
		tprints_arg_name("clockid");
		printclockname(tcp->u_arg[0]);

		/* sevp */
		tprints_arg_next_name("sevp");
		print_sigevent(tcp, tcp->u_arg[1]);
	} else {
		/* timerid */
		tprints_arg_next_name("timerid");
		printnum_int(tcp, tcp->u_arg[2], "%d");
	}
	return 0;
}

SYS_FUNC(timer_delete)
{
	tprints_arg_name("timerid");
	PRINT_VAL_D((int) tcp->u_arg[0]);

	return RVAL_DECODED;
}

static int
do_timer_settime(struct tcb *const tcp, const print_obj_by_addr_fn print_its)
{
	if (entering(tcp)) {
		/* timerid */
		tprints_arg_name("timerid");
		PRINT_VAL_D((int) tcp->u_arg[0]);

		/* flags */
		tprints_arg_next_name("flags");
		printflags(clockflags, tcp->u_arg[1], "TIMER_???");

		/* new_value */
		tprints_arg_next_name("new_value");
		print_its(tcp, tcp->u_arg[2]);
	} else {
		/* old_value */
		tprints_arg_next_name("old_value");
		print_its(tcp, tcp->u_arg[3]);
	}
	return 0;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(timer_settime32)
{
	return do_timer_settime(tcp, print_itimerspec32);
}
#endif

SYS_FUNC(timer_settime64)
{
	return do_timer_settime(tcp, print_itimerspec64);
}

static int
do_timer_gettime(struct tcb *const tcp, const print_obj_by_addr_fn print_its)
{
	if (entering(tcp)) {
		/* timerid */
		tprints_arg_name("timerid");
		PRINT_VAL_D((int) tcp->u_arg[0]);
	} else {
		/* curr_value */
		tprints_arg_next_name("curr_value");
		print_its(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(timer_gettime32)
{
	return do_timer_gettime(tcp, print_itimerspec32);
}
#endif

SYS_FUNC(timer_gettime64)
{
	return do_timer_gettime(tcp, print_itimerspec64);
}

#include "xlat/timerfdflags.h"

SYS_FUNC(timerfd_create)
{
	/* clockid */
	tprints_arg_name("clockid");
	printclockname(tcp->u_arg[0]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(timerfdflags, tcp->u_arg[1], "TFD_???");

	return RVAL_DECODED | RVAL_FD;
}

static int
do_timerfd_settime(struct tcb *const tcp, const print_obj_by_addr_fn print_its)
{
	if (entering(tcp)) {
		/* fd */
		tprints_arg_name("fd");
		printfd(tcp, tcp->u_arg[0]);

		/* flags */
		tprints_arg_next_name("flags");
		printflags(timerfdflags, tcp->u_arg[1], "TFD_???");

		/* new_value */
		tprints_arg_next_name("new_value");
		print_its(tcp, tcp->u_arg[2]);
	} else {
		/* old_value */
		tprints_arg_next_name("old_value");
		print_its(tcp, tcp->u_arg[3]);
	}
	return 0;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(timerfd_settime32)
{
	return do_timerfd_settime(tcp, print_itimerspec32);
}
#endif

SYS_FUNC(timerfd_settime64)
{
	return do_timerfd_settime(tcp, print_itimerspec64);
}

static int
do_timerfd_gettime(struct tcb *const tcp, const print_obj_by_addr_fn print_its)
{
	if (entering(tcp)) {
		/* fd */
		tprints_arg_name("fd");
		printfd(tcp, tcp->u_arg[0]);
	} else {
		/* curr_value */
		tprints_arg_next_name("curr_value");
		print_its(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(timerfd_gettime32)
{
	return do_timerfd_gettime(tcp, print_itimerspec32);
}
#endif

SYS_FUNC(timerfd_gettime64)
{
	return do_timerfd_gettime(tcp, print_itimerspec64);
}
