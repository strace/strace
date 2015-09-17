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

SYS_FUNC(getitimer)
{
	if (entering(tcp)) {
		printxval(itimer_which, tcp->u_arg[0], "ITIMER_???");
		tprints(", ");
	} else {
		print_itimerval(tcp, tcp->u_arg[1]);
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
		print_itimerval32(tcp, tcp->u_arg[1]);
	}
	return 0;
}
#endif

SYS_FUNC(setitimer)
{
	if (entering(tcp)) {
		printxval(itimer_which, tcp->u_arg[0], "ITIMER_???");
		tprints(", ");
		print_itimerval(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
		print_itimerval(tcp, tcp->u_arg[2]);
	}
	return 0;
}

#ifdef ALPHA
SYS_FUNC(osf_setitimer)
{
	if (entering(tcp)) {
		printxval(itimer_which, tcp->u_arg[0], "ITIMER_???");
		tprints(", ");
		print_itimerval32(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
		print_itimerval32(tcp, tcp->u_arg[2]);
	}
	return 0;
}
#endif

#include "xlat/adjtimex_state.h"

static int
do_adjtimex(struct tcb *tcp, long addr)
{
	if (print_timex(tcp, addr))
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

SYS_FUNC(timer_create)
{
	if (entering(tcp)) {
		printclockname(tcp->u_arg[0]);
		tprints(", ");
		print_sigevent(tcp, tcp->u_arg[1]);
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
		print_itimerspec(tcp, tcp->u_arg[2]);
		tprints(", ");
	} else {
		print_itimerspec(tcp, tcp->u_arg[3]);
	}
	return 0;
}

SYS_FUNC(timer_gettime)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		print_itimerspec(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#include "xlat/timerfdflags.h"

SYS_FUNC(timerfd)
{
	tprintf("%ld, ", tcp->u_arg[0]);
	printclockname(tcp->u_arg[0]);
	tprints(", ");
	printflags(timerfdflags, tcp->u_arg[2], "TFD_???");
	tprints(", ");
	print_itimerspec(tcp, tcp->u_arg[3]);

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(timerfd_create)
{
	printclockname(tcp->u_arg[0]);
	tprints(", ");
	printflags(timerfdflags, tcp->u_arg[1], "TFD_???");

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(timerfd_settime)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printflags(timerfdflags, tcp->u_arg[1], "TFD_???");
	tprints(", ");
	print_itimerspec(tcp, tcp->u_arg[2]);
	tprints(", ");
	print_itimerspec(tcp, tcp->u_arg[3]);

	return RVAL_DECODED;
}

SYS_FUNC(timerfd_gettime)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_itimerspec(tcp, tcp->u_arg[1]);
	}
	return 0;
}
