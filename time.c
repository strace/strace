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

#include "defs.h"
#include <fcntl.h>
#include <signal.h>
#include <linux/version.h>
#include <sys/timex.h>

#ifndef UTIME_NOW
#define UTIME_NOW ((1l << 30) - 1l)
#endif
#ifndef UTIME_OMIT
#define UTIME_OMIT ((1l << 30) - 2l)
#endif

#if SUPPORTED_PERSONALITIES > 1
# if defined X86_64 || defined X32
#  define current_time_t_is_compat (current_personality == 1)
# else
#  define current_time_t_is_compat (current_wordsize == 4)
# endif
#else
# define current_time_t_is_compat 0
#endif

struct timeval32
{
	u_int32_t tv_sec, tv_usec;
};

static void
tprint_timeval32(struct tcb *tcp, const struct timeval32 *tv)
{
	tprintf("{%u, %u}", tv->tv_sec, tv->tv_usec);
}

static void
tprint_timeval(struct tcb *tcp, const struct timeval *tv)
{
	tprintf("{%ju, %ju}", (uintmax_t) tv->tv_sec, (uintmax_t) tv->tv_usec);
}

void
printtv_bitness(struct tcb *tcp, long addr, enum bitness_t bitness, int special)
{
	char buf[TIMEVAL_TEXT_BUFSIZE];
	sprinttv(buf, tcp, addr, bitness, special);
	tprints(buf);
}

static char *
do_sprinttv(char *buf, const uintmax_t sec, const uintmax_t usec,
	    const int special)
{
	if (special) {
		switch (usec) {
			case UTIME_NOW:
				return stpcpy(buf, "UTIME_NOW");
			case UTIME_OMIT:
				return stpcpy(buf, "UTIME_OMIT");
		}
	}
	return buf + sprintf(buf, "{%ju, %ju}", sec, usec);
}

char *
sprinttv(char *buf, struct tcb *tcp, long addr, enum bitness_t bitness, int special)
{
	if (addr == 0)
		return stpcpy(buf, "NULL");

	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)))
		return buf + sprintf(buf, "%#lx", addr);

	if (bitness == BITNESS_32 || current_time_t_is_compat)
	{
		struct timeval32 tv;

		if (umove(tcp, addr, &tv) >= 0)
			return do_sprinttv(buf, tv.tv_sec, tv.tv_usec, special);
	} else {
		struct timeval tv;

		if (umove(tcp, addr, &tv) >= 0)
			return do_sprinttv(buf, tv.tv_sec, tv.tv_usec, special);
	}

	return buf + sprintf(buf, "%#lx", addr);
}

void
print_timespec(struct tcb *tcp, long addr)
{
	char buf[TIMESPEC_TEXT_BUFSIZE];
	sprint_timespec(buf, tcp, addr);
	tprints(buf);
}

void
sprint_timespec(char *buf, struct tcb *tcp, long addr)
{
	if (addr == 0)
		strcpy(buf, "NULL");
	else if (!verbose(tcp))
		sprintf(buf, "%#lx", addr);
	else {
		int rc;

#if SUPPORTED_PERSONALITIES > 1
		if (current_time_t_is_compat) {
			struct timeval32 tv;

			rc = umove(tcp, addr, &tv);
			if (rc >= 0)
				sprintf(buf, "{%u, %u}",
					tv.tv_sec, tv.tv_usec);
		} else
#endif
		{
			struct timespec ts;

			rc = umove(tcp, addr, &ts);
			if (rc >= 0)
				sprintf(buf, "{%ju, %ju}",
					(uintmax_t) ts.tv_sec,
					(uintmax_t) ts.tv_nsec);
		}
		if (rc < 0)
			strcpy(buf, "{...}");
	}
}

SYS_FUNC(time)
{
	if (exiting(tcp)) {
		printnum_long(tcp, tcp->u_arg[0], "%ld");
	}
	return 0;
}

SYS_FUNC(gettimeofday)
{
	if (exiting(tcp)) {
		printtv(tcp, tcp->u_arg[0]);
		tprints(", ");
		printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef ALPHA
SYS_FUNC(osf_gettimeofday)
{
	if (exiting(tcp)) {
		printtv_bitness(tcp, tcp->u_arg[0], BITNESS_32, 0);
		tprints(", ");
		printtv_bitness(tcp, tcp->u_arg[1], BITNESS_32, 0);
	}
	return 0;
}
#endif

SYS_FUNC(settimeofday)
{
	printtv(tcp, tcp->u_arg[0]);
	tprints(", ");
	printtv(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

#ifdef ALPHA
SYS_FUNC(osf_settimeofday)
{
	printtv_bitness(tcp, tcp->u_arg[0], BITNESS_32, 0);
	tprints(", ");
	printtv_bitness(tcp, tcp->u_arg[1], BITNESS_32, 0);

	return RVAL_DECODED;
}
#endif

SYS_FUNC(adjtime)
{
	if (entering(tcp)) {
		printtv(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(nanosleep)
{
	if (entering(tcp)) {
		print_timespec(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		/* Second (returned) timespec is only significant
		 * if syscall was interrupted. On success, we print
		 * only its address, since kernel doesn't modify it,
		 * and printing the value may show uninitialized data.
		 */
		switch (tcp->u_error) {
		default:
			/* Not interrupted (slept entire interval) */
			printaddr(tcp->u_arg[1]);
			break;
		case ERESTARTSYS:
		case ERESTARTNOINTR:
		case ERESTARTNOHAND:
		case ERESTART_RESTARTBLOCK:
			/* Interrupted */
			print_timespec(tcp, tcp->u_arg[1]);
		}
	}
	return 0;
}

#include "xlat/itimer_which.h"

static void
printitv_bitness(struct tcb *tcp, long addr, enum bitness_t bitness)
{
	if (bitness == BITNESS_32 || current_time_t_is_compat) {
		struct {
			struct timeval32 it_interval, it_value;
		} itv;

		if (!umove_or_printaddr(tcp, addr, &itv)) {
			tprints("{it_interval=");
			tprint_timeval32(tcp, &itv.it_interval);
			tprints(", it_value=");
			tprint_timeval32(tcp, &itv.it_value);
			tprints("}");
		}
	} else {
		struct itimerval itv;

		if (!umove_or_printaddr(tcp, addr, &itv)) {
			tprints("{it_interval=");
			tprint_timeval(tcp, &itv.it_interval);
			tprints(", it_value=");
			tprint_timeval(tcp, &itv.it_value);
			tprints("}");
		}
	}
}

#define printitv(tcp, addr)	\
	printitv_bitness((tcp), (addr), BITNESS_CURRENT)

SYS_FUNC(getitimer)
{
	if (entering(tcp)) {
		printxval(itimer_which, tcp->u_arg[0], "ITIMER_???");
		tprints(", ");
	} else {
		printitv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef ALPHA
SYS_FUNC(osf_getitimer)
{
	if (entering(tcp)) {
		printxval(itimer_which, tcp->u_arg[0], "ITIMER_???");
		tprints(", ");
	} else {
		printitv_bitness(tcp, tcp->u_arg[1], BITNESS_32);
	}
	return 0;
}
#endif

SYS_FUNC(setitimer)
{
	if (entering(tcp)) {
		printxval(itimer_which, tcp->u_arg[0], "ITIMER_???");
		tprints(", ");
		printitv(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
		printitv(tcp, tcp->u_arg[2]);
	}
	return 0;
}

#ifdef ALPHA
SYS_FUNC(osf_setitimer)
{
	if (entering(tcp)) {
		printxval(itimer_which, tcp->u_arg[0], "ITIMER_???");
		tprints(", ");
		printitv_bitness(tcp, tcp->u_arg[1], BITNESS_32);
		tprints(", ");
	} else {
		printitv_bitness(tcp, tcp->u_arg[2], BITNESS_32);
	}
	return 0;
}
#endif

#include "xlat/adjtimex_modes.h"
#include "xlat/adjtimex_status.h"
#include "xlat/adjtimex_state.h"

#if SUPPORTED_PERSONALITIES > 1
static int
tprint_timex32(struct tcb *tcp, long addr)
{
	struct {
		unsigned int modes;
		int     offset;
		int     freq;
		int     maxerror;
		int     esterror;
		int     status;
		int     constant;
		int     precision;
		int     tolerance;
		struct timeval32 time;
		int     tick;
		int     ppsfreq;
		int     jitter;
		int     shift;
		int     stabil;
		int     jitcnt;
		int     calcnt;
		int     errcnt;
		int     stbcnt;
	} tx;

	if (umove_or_printaddr(tcp, addr, &tx))
		return -1;

	tprints("{modes=");
	printflags(adjtimex_modes, tx.modes, "ADJ_???");
	tprintf(", offset=%d, freq=%d, maxerror=%d, ",
		tx.offset, tx.freq, tx.maxerror);
	tprintf("esterror=%u, status=", tx.esterror);
	printflags(adjtimex_status, tx.status, "STA_???");
	tprintf(", constant=%d, precision=%u, ",
		tx.constant, tx.precision);
	tprintf("tolerance=%d, time=", tx.tolerance);
	tprint_timeval32(tcp, &tx.time);
	tprintf(", tick=%d, ppsfreq=%d, jitter=%d",
		tx.tick, tx.ppsfreq, tx.jitter);
	tprintf(", shift=%d, stabil=%d, jitcnt=%d",
		tx.shift, tx.stabil, tx.jitcnt);
	tprintf(", calcnt=%d, errcnt=%d, stbcnt=%d",
		tx.calcnt, tx.errcnt, tx.stbcnt);
	tprints("}");
	return 0;
}
#endif /* SUPPORTED_PERSONALITIES > 1 */

static int
tprint_timex(struct tcb *tcp, long addr)
{
	struct timex tx;

#if SUPPORTED_PERSONALITIES > 1
	if (current_time_t_is_compat)
		return tprint_timex32(tcp, addr);
#endif
	if (umove_or_printaddr(tcp, addr, &tx))
		return -1;

#if LINUX_VERSION_CODE < 66332
	tprintf("{mode=%d, offset=%ld, frequency=%ld, ",
		tx.mode, tx.offset, tx.frequency);
	tprintf("maxerror=%ld, esterror=%lu, status=%u, ",
		tx.maxerror, tx.esterror, tx.status);
	tprintf("time_constant=%ld, precision=%lu, ",
		tx.time_constant, tx.precision);
	tprintf("tolerance=%ld, time=", tx.tolerance);
	tprint_timeval(tcp, &tx.time);
#else
	tprints("{modes=");
	printflags(adjtimex_modes, tx.modes, "ADJ_???");
	tprintf(", offset=%jd, freq=%jd, maxerror=%ju, esterror=%ju, status=",
		(intmax_t) tx.offset, (intmax_t) tx.freq,
		(uintmax_t) tx.maxerror, (uintmax_t) tx.esterror);
	printflags(adjtimex_status, tx.status, "STA_???");
	tprintf(", constant=%jd, precision=%ju, tolerance=%jd, time=",
		(intmax_t) tx.constant, (uintmax_t) tx.precision,
		(intmax_t) tx.tolerance);
	tprint_timeval(tcp, &tx.time);
	tprintf(", tick=%jd, ppsfreq=%jd, jitter=%jd",
		(intmax_t) tx.tick, (intmax_t) tx.ppsfreq, (intmax_t) tx.jitter);
	tprintf(", shift=%d, stabil=%jd, jitcnt=%jd",
		tx.shift, (intmax_t) tx.stabil, (intmax_t) tx.jitcnt);
	tprintf(", calcnt=%jd, errcnt=%jd, stbcnt=%jd",
		(intmax_t) tx.calcnt, (intmax_t) tx.errcnt, (intmax_t) tx.stbcnt);
#endif
	tprints("}");
	return 0;
}

static int
do_adjtimex(struct tcb *tcp, long addr)
{
	if (tprint_timex(tcp, addr))
		return 0;
	tcp->auxstr = xlookup(adjtimex_state, tcp->u_rval);
	if (tcp->auxstr)
		return RVAL_STR;
	return 0;
}

SYS_FUNC(adjtimex)
{
	if (exiting(tcp))
		return do_adjtimex(tcp, tcp->u_arg[0]);
	return 0;
}

#include "xlat/clockflags.h"
#include "xlat/clocknames.h"

static void
printclockname(int clockid)
{
#ifdef CLOCKID_TO_FD
# include "xlat/cpuclocknames.h"

	if (clockid < 0) {
		if ((clockid & CLOCKFD_MASK) == CLOCKFD)
			tprintf("FD_TO_CLOCKID(%d)", CLOCKID_TO_FD(clockid));
		else {
			if(CPUCLOCK_PERTHREAD(clockid))
				tprintf("MAKE_THREAD_CPUCLOCK(%d,", CPUCLOCK_PID(clockid));
			else
				tprintf("MAKE_PROCESS_CPUCLOCK(%d,", CPUCLOCK_PID(clockid));
			printxval(cpuclocknames, clockid & CLOCKFD_MASK, "CPUCLOCK_???");
			tprints(")");
		}
	}
	else
#endif
		printxval(clocknames, clockid, "CLOCK_???");
}

SYS_FUNC(clock_settime)
{
	printclockname(tcp->u_arg[0]);
	tprints(", ");
	printtv(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(clock_gettime)
{
	if (entering(tcp)) {
		printclockname(tcp->u_arg[0]);
		tprints(", ");
	} else {
		printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(clock_nanosleep)
{
	if (entering(tcp)) {
		printclockname(tcp->u_arg[0]);
		tprints(", ");
		printflags(clockflags, tcp->u_arg[1], "TIMER_???");
		tprints(", ");
		printtv(tcp, tcp->u_arg[2]);
		tprints(", ");
	} else {
		printtv(tcp, tcp->u_arg[3]);
	}
	return 0;
}

SYS_FUNC(clock_adjtime)
{
	if (exiting(tcp))
		return do_adjtimex(tcp, tcp->u_arg[1]);
	printclockname(tcp->u_arg[0]);
	tprints(", ");
	return 0;
}

#ifndef SIGEV_THREAD_ID
# define SIGEV_THREAD_ID 4
#endif
#include "xlat/sigev_value.h"

#if SUPPORTED_PERSONALITIES > 1
static void
printsigevent32(struct tcb *tcp, long arg)
{
	struct {
		int     sigev_value;
		int     sigev_signo;
		int     sigev_notify;

		union {
			int     tid;
			struct {
				int     function, attribute;
			} thread;
		} un;
	} sev;

	if (!umove_or_printaddr(tcp, arg, &sev)) {
		tprintf("{%#x, ", sev.sigev_value);
		if (sev.sigev_notify == SIGEV_SIGNAL)
			tprintf("%s, ", signame(sev.sigev_signo));
		else
			tprintf("%u, ", sev.sigev_signo);
		printxval(sigev_value, sev.sigev_notify, "SIGEV_???");
		tprints(", ");
		if (sev.sigev_notify == SIGEV_THREAD_ID)
			tprintf("{%d}", sev.un.tid);
		else if (sev.sigev_notify == SIGEV_THREAD)
			tprintf("{%#x, %#x}",
				sev.un.thread.function,
				sev.un.thread.attribute);
		else
			tprints("{...}");
		tprints("}");
	}
}
#endif

void
printsigevent(struct tcb *tcp, long arg)
{
	struct sigevent sev;

#if SUPPORTED_PERSONALITIES > 1
	if (current_wordsize == 4) {
		printsigevent32(tcp, arg);
		return;
	}
#endif
	if (!umove_or_printaddr(tcp, arg, &sev)) {
		tprintf("{%p, ", sev.sigev_value.sival_ptr);
		if (sev.sigev_notify == SIGEV_SIGNAL)
			tprintf("%s, ", signame(sev.sigev_signo));
		else
			tprintf("%u, ", sev.sigev_signo);
		printxval(sigev_value, sev.sigev_notify, "SIGEV_???");
		tprints(", ");
		if (sev.sigev_notify == SIGEV_THREAD_ID)
#if defined(HAVE_STRUCT_SIGEVENT__SIGEV_UN__PAD)
			/* _pad[0] is the _tid field which might not be
			   present in the userlevel definition of the
			   struct.  */
			tprintf("{%d}", sev._sigev_un._pad[0]);
#elif defined(HAVE_STRUCT_SIGEVENT___PAD)
			tprintf("{%d}", sev.__pad[0]);
#else
# warning unfamiliar struct sigevent => incomplete SIGEV_THREAD_ID decoding
			tprints("{...}");
#endif
		else if (sev.sigev_notify == SIGEV_THREAD)
			tprintf("{%p, %p}", sev.sigev_notify_function,
				sev.sigev_notify_attributes);
		else
			tprints("{...}");
		tprints("}");
	}
}

SYS_FUNC(timer_create)
{
	if (entering(tcp)) {
		printclockname(tcp->u_arg[0]);
		tprints(", ");
		printsigevent(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
		printnum_int(tcp, tcp->u_arg[2], "%d");
	}
	return 0;
}

SYS_FUNC(timer_settime)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		printflags(clockflags, tcp->u_arg[1], "TIMER_???");
		tprints(", ");
		printitv(tcp, tcp->u_arg[2]);
		tprints(", ");
	} else {
		printitv(tcp, tcp->u_arg[3]);
	}
	return 0;
}

SYS_FUNC(timer_gettime)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		printitv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#include "xlat/timerfdflags.h"

SYS_FUNC(timerfd)
{
	/* It does not matter that the kernel uses itimerspec.  */
	tprintf("%ld, ", tcp->u_arg[0]);
	printclockname(tcp->u_arg[0]);
	tprints(", ");
	printflags(timerfdflags, tcp->u_arg[2], "TFD_???");
	tprints(", ");
	printitv(tcp, tcp->u_arg[3]);

	return RVAL_DECODED;
}

SYS_FUNC(timerfd_create)
{
	printclockname(tcp->u_arg[0]);
	tprints(", ");
	printflags(timerfdflags, tcp->u_arg[1], "TFD_???");

	return RVAL_DECODED;
}

SYS_FUNC(timerfd_settime)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printflags(timerfdflags, tcp->u_arg[1], "TFD_???");
	tprints(", ");
	printitv(tcp, tcp->u_arg[2]);
	tprints(", ");
	printitv(tcp, tcp->u_arg[3]);

	return RVAL_DECODED;
}

SYS_FUNC(timerfd_gettime)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printitv(tcp, tcp->u_arg[1]);
	}
	return 0;
}
