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
 *
 *	$Id$
 */

#include "defs.h"

#ifdef LINUX
#include <linux/version.h>
#include <sys/timex.h>
#include <linux/ioctl.h>
#include <linux/rtc.h>
#endif /* LINUX */

void
printtv(tcp, addr)
struct tcb *tcp;
long addr;
{
	struct timeval tv;

	if (addr == 0)
		tprintf("NULL");
	else if (!verbose(tcp))
		tprintf("%#lx", addr);
	else if (umove(tcp, addr, &tv) < 0)
		tprintf("{...}");
	else
		tprintf("{%lu, %lu}", (long) tv.tv_sec, (long) tv.tv_usec);
}

#ifdef ALPHA
struct timeval32
{
    unsigned tv_sec;
    unsigned tv_usec;
};

void
printtv32(tcp, addr)
struct tcb *tcp;
long addr;
{
    struct timeval32  tv;

    if (addr == 0)
	tprintf("NULL");
    else if (!verbose(tcp))
	tprintf("%#lx", addr);
    else if (umove(tcp, addr, &tv) < 0)
	tprintf("{...}");
    else
	tprintf("{%u, %u}", tv.tv_sec, tv.tv_usec);
}
#endif


int
sys_time(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
#ifndef SVR4
		printnum(tcp, tcp->u_arg[0], "%ld");
#endif /* SVR4 */
	}
	return 0;
}

int
sys_stime(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		printnum(tcp, tcp->u_arg[0], "%ld");
	}
	return 0;
}

int
sys_gettimeofday(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		if (syserror(tcp)) {
			tprintf("%#lx, %#lx",
				tcp->u_arg[0], tcp->u_arg[1]);
			return 0;
		}
		printtv(tcp, tcp->u_arg[0]);
#ifndef SVR4
		tprintf(", ");
		printtv(tcp, tcp->u_arg[1]);
#endif /* !SVR4 */
	}
	return 0;
}


#ifdef ALPHA
int
sys_osf_gettimeofday(tcp)
struct tcb *tcp;
{
    if (exiting(tcp)) {
	if (syserror(tcp)) {
	    tprintf("%#lx, %#lx",
		    tcp->u_arg[0], tcp->u_arg[1]);
	    return 0;
	}
	printtv32(tcp, tcp->u_arg[0]);
#ifndef SVR4
	tprintf(", ");
	printtv32(tcp, tcp->u_arg[1]);
#endif /* !SVR4 */
    }
    return 0;
}
#endif

int
sys_settimeofday(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printtv(tcp, tcp->u_arg[0]);
#ifndef SVR4
		tprintf(", ");
		printtv(tcp, tcp->u_arg[1]);
#endif /* !SVR4 */
	}
	return 0;
}

#ifdef ALPHA
int
sys_osf_settimeofday(tcp)
struct tcb *tcp;
{
    if (entering(tcp)) {
	printtv32(tcp, tcp->u_arg[0]);
#ifndef SVR4
	tprintf(", ");
	printtv32(tcp, tcp->u_arg[1]);
#endif /* !SVR4 */
    }
    return 0;
}
#endif

int
sys_adjtime(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printtv(tcp, tcp->u_arg[0]);
		tprintf(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

static const struct xlat which[] = {
	{ ITIMER_REAL,	"ITIMER_REAL"	},
	{ ITIMER_VIRTUAL,"ITIMER_VIRTUAL"},
	{ ITIMER_PROF,	"ITIMER_PROF"	},
	{ 0,		NULL		},
};

static void
printitv(tcp, addr)
struct tcb *tcp;
long addr;
{
	struct itimerval itv;

	if (addr == 0)
		tprintf("NULL");
	else if (!verbose(tcp))
		tprintf("%#lx", addr);
	else if (umove(tcp, addr, &itv) < 0)
		tprintf("{...}");
	else {
		tprintf("{it_interval={%lu, %lu}, it_value={%lu, %lu}}",
		(long) itv.it_interval.tv_sec, (long) itv.it_interval.tv_usec,
		(long) itv.it_value.tv_sec, (long) itv.it_value.tv_usec);
	}
}


#ifdef ALPHA
static void
printitv32(tcp, addr)
struct tcb *tcp;
long addr;
{
    struct itimerval32
    {
	struct timeval32 it_interval;
	struct timeval32 it_value;
    } itv;

    if (addr == 0)
	tprintf("NULL");
    else if (!verbose(tcp))
	tprintf("%#lx", addr);
    else if (umove(tcp, addr, &itv) < 0)
	tprintf("{...}");
    else {
	tprintf("{it_interval={%u, %u}, it_value={%u, %u}}",
		itv.it_interval.tv_sec, itv.it_interval.tv_usec,
		itv.it_value.tv_sec, itv.it_value.tv_usec);
    }
}
#endif

int
sys_getitimer(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(which, tcp->u_arg[0], "ITIMER_???");
		tprintf(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printitv(tcp, tcp->u_arg[1]);
	}
	return 0;
}


#ifdef ALPHA
int
sys_osf_getitimer(tcp)
struct tcb *tcp;
{
    if (entering(tcp)) {
	printxval(which, tcp->u_arg[0], "ITIMER_???");
	tprintf(", ");
    } else {
	if (syserror(tcp))
	    tprintf("%#lx", tcp->u_arg[1]);
	else
	    printitv32(tcp, tcp->u_arg[1]);
    }
    return 0;
}
#endif

int
sys_setitimer(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(which, tcp->u_arg[0], "ITIMER_???");
		tprintf(", ");
		printitv(tcp, tcp->u_arg[1]);
		tprintf(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[2]);
		else
			printitv(tcp, tcp->u_arg[2]);
	}
	return 0;
}

#ifdef ALPHA
int
sys_osf_setitimer(tcp)
struct tcb *tcp;
{
    if (entering(tcp)) {
	printxval(which, tcp->u_arg[0], "ITIMER_???");
	tprintf(", ");
	printitv32(tcp, tcp->u_arg[1]);
	tprintf(", ");
    } else {
	if (syserror(tcp))
	    tprintf("%#lx", tcp->u_arg[2]);
	else
	    printitv32(tcp, tcp->u_arg[2]);
    }
    return 0;
}
#endif

#ifdef LINUX

int
sys_adjtimex(tcp)
struct tcb *tcp;
{
	struct timex txc;

	if (exiting(tcp)) {
		if (tcp->u_arg[0] == 0)
			tprintf("NULL");
		else if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (umove(tcp, tcp->u_arg[0], &txc) < 0)
			tprintf("{...}");
		else {
#if LINUX_VERSION_CODE < 66332
			tprintf("{mode=%d, offset=%ld, frequency=%ld, ",
				txc.mode, txc.offset, txc.frequency);
			tprintf("maxerror=%ld, esterror=%lu, status=%u, ",
				txc.maxerror, txc.esterror, txc.status);
			tprintf("time_constant=%ld, precision=%lu, ",
				txc.time_constant, txc.precision);
			tprintf("tolerance=%ld, time={%lu, %lu}}",
				txc.tolerance, (long) txc.time.tv_sec,
				(long) txc.time.tv_usec);
#else
			tprintf("{modes=%d, offset=%ld, freq=%ld, ",
				txc.modes, txc.offset, txc.freq);
			tprintf("maxerror=%ld, esterror=%lu, status=%u, ",
				txc.maxerror, txc.esterror, txc.status);
			tprintf("constant=%ld, precision=%lu, ",
				txc.constant, txc.precision);
			tprintf("tolerance=%ld, time={%lu, %lu}}",
				txc.tolerance, (long) txc.time.tv_sec,
				(long) txc.time.tv_usec);
			/* there's a bunch of other stuff, but it's not
			 * worth the time or the trouble to include */
#endif
		}
	}
	return 0;
}

static const struct xlat clockflags[] = {
  { TIMER_ABSTIME, "TIMER_ABSTIME" },
  { 0,             NULL }
};

static const struct xlat clocknames[] = {
#ifdef CLOCK_REALTIME
  { CLOCK_REALTIME, "CLOCK_REALTIME" },
#endif
#ifdef CLOCK_MONOTONIC
  { CLOCK_MONOTONIC, "CLOCK_MONOTONIC" },
#endif
  { 0,             NULL }
};

int
sys_clock_settime(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(clocknames, tcp->u_arg[0], "CLOCK_???");
		tprintf(", ");
		printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_clock_gettime(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(clocknames, tcp->u_arg[0], "CLOCK_???");
		tprintf(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printtv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_clock_nanosleep(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(clocknames, tcp->u_arg[0], "CLOCK_???");
		tprintf(", ");
		printflags(clockflags, tcp->u_arg[1], "TIMER_???");
		tprintf(", ");
		printtv(tcp, tcp->u_arg[2]);
		tprintf(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[3]);
		else
			printtv(tcp, tcp->u_arg[3]);
	}
	return 0;
}

#ifndef SIGEV_THREAD_ID
# define SIGEV_THREAD_ID 4
#endif
static const struct xlat sigev_value[] = {
	{ SIGEV_SIGNAL+1, "SIGEV_SIGNAL" },
	{ SIGEV_NONE+1, "SIGEV_NONE" },
	{ SIGEV_THREAD+1, "SIGEV_THREAD" },
	{ SIGEV_THREAD_ID+1, "SIGEV_THREAD_ID" },
	{ 0, NULL }
};

void
printsigevent(tcp, arg)
struct tcb *tcp;
long arg;
{
	struct sigevent sev;
	if (umove (tcp, arg, &sev) < 0)
		tprintf("{...}");
	else {
		tprintf("{%p, ", sev.sigev_value.sival_ptr);
		if (sev.sigev_notify == SIGEV_SIGNAL)
			tprintf("%s, ", signame(sev.sigev_signo));
		else
			tprintf("%u, ", sev.sigev_signo);
		printxval(sigev_value, sev.sigev_notify+1, "SIGEV_???");
		tprintf(", ");
		if (sev.sigev_notify == SIGEV_THREAD_ID)
			/* _pad[0] is the _tid field which might not be
			   present in the userlevel definition of the
			   struct.  */
			tprintf("{%d}", sev._sigev_un._pad[0]);
		else if (sev.sigev_notify == SIGEV_THREAD)
			tprintf("{%p, %p}", sev.sigev_notify_function,
				sev.sigev_notify_attributes);
		else
			tprintf("{...}");
		tprintf("}");
	}
}

int
sys_timer_create(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(clocknames, tcp->u_arg[0], "CLOCK_???");
		tprintf(", ");
		printsigevent(tcp, tcp->u_arg[1]);
		tprintf(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[2]);
		else {
			void *p;
			umove(tcp, tcp->u_arg[2], &p);
			tprintf("{%p}", p);
		}
	}
	return 0;
}

int
sys_timer_settime(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%#lx, ", tcp->u_arg[0]);
		printflags(clockflags, tcp->u_arg[1], "TIMER_???");
		tprintf(", ");
		printitv(tcp, tcp->u_arg[2]);
		tprintf(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[3]);
		else
			printitv(tcp, tcp->u_arg[3]);
	}
	return 0;
}

int
sys_timer_gettime(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%#lx, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printitv(tcp, tcp->u_arg[1]);
	}
	return 0;
}

static void
print_rtc(tcp, rt)
struct tcb *tcp;
const struct rtc_time *rt;
{
	tprintf("{tm_sec=%d, tm_min=%d, tm_hour=%d, "
		"tm_mday=%d, tm_mon=%d, tm_year=%d, ",
		rt->tm_sec, rt->tm_min, rt->tm_hour,
		rt->tm_mday, rt->tm_mon, rt->tm_year);
	if (!abbrev(tcp))
		tprintf("tm_wday=%d, tm_yday=%d, tm_isdst=%d}",
			rt->tm_wday, rt->tm_yday, rt->tm_isdst);
	else
		tprintf("...}");
}

int
rtc_ioctl(tcp, code, arg)
struct tcb *tcp;
long code;
long arg;
{
	switch (code) {
	case RTC_ALM_SET:
	case RTC_SET_TIME:
		if (entering(tcp)) {
			struct rtc_time rt;
			if (umove(tcp, arg, &rt) < 0)
				tprintf(", %#lx", arg);
			else {
				tprintf(", ");
				print_rtc(tcp, &rt);
			}
		}
		break;
	case RTC_ALM_READ:
	case RTC_RD_TIME:
		if (exiting(tcp)) {
			struct rtc_time rt;
			if (syserror(tcp) || umove(tcp, arg, &rt) < 0)
				tprintf(", %#lx", arg);
			else {
				tprintf(", ");
				print_rtc(tcp, &rt);
			}
		}
		break;
	case RTC_IRQP_SET:
	case RTC_EPOCH_SET:
		if (entering(tcp))
			tprintf(", %lu", arg);
		break;
	case RTC_IRQP_READ:
	case RTC_EPOCH_READ:
		if (exiting(tcp))
			tprintf(", %lu", arg);
		break;
	case RTC_WKALM_SET:
		if (entering(tcp)) {
			struct rtc_wkalrm wk;
			if (umove(tcp, arg, &wk) < 0)
				tprintf(", %#lx", arg);
			else {
				tprintf(", {enabled=%d, pending=%d, ",
					wk.enabled, wk.pending);
				print_rtc(tcp, &wk.time);
				tprintf("}");
			}
		}
		break;
	case RTC_WKALM_RD:
		if (exiting(tcp)) {
			struct rtc_wkalrm wk;
			if (syserror(tcp) || umove(tcp, arg, &wk) < 0)
				tprintf(", %#lx", arg);
			else {
				tprintf(", {enabled=%d, pending=%d, ",
					wk.enabled, wk.pending);
				print_rtc(tcp, &wk.time);
				tprintf("}");
			}
		}
		break;
	default:
		if (entering(tcp))
			tprintf(", %#lx", arg);
		break;
	}
	return 1;
}
#endif /* LINUX */
