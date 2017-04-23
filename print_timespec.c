/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include DEF_MPERS_TYPE(timespec_t)

typedef struct timespec timespec_t;

#include MPERS_DEFS

#ifndef UTIME_NOW
# define UTIME_NOW ((1l << 30) - 1l)
#endif
#ifndef UTIME_OMIT
# define UTIME_OMIT ((1l << 30) - 2l)
#endif

static const char timespec_fmt[] = "{tv_sec=%lld, tv_nsec=%llu}";

static void
print_timespec_t(const timespec_t *t)
{
	tprintf(timespec_fmt, (long long) t->tv_sec,
		zero_extend_signed_to_ull(t->tv_nsec));
}

static void
print_timespec_t_utime(const timespec_t *t)
{
	switch (t->tv_nsec) {
	case UTIME_NOW:
		tprints("UTIME_NOW");
		break;
	case UTIME_OMIT:
		tprints("UTIME_OMIT");
		break;
	default:
		print_timespec_t(t);
		tprints_comment(sprinttime_nsec(t->tv_sec,
			zero_extend_signed_to_ull(t->tv_nsec)));
		break;
	}
}

MPERS_PRINTER_DECL(void, print_timespec,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	timespec_t t;

	if (umove_or_printaddr(tcp, addr, &t))
		return;

	print_timespec_t(&t);
}

MPERS_PRINTER_DECL(const char *, sprint_timespec,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	timespec_t t;
	static char buf[sizeof(timespec_fmt) + 3 * sizeof(t)];

	if (!addr) {
		strcpy(buf, "NULL");
	} else if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
		   umove(tcp, addr, &t)) {
		snprintf(buf, sizeof(buf), "%#" PRI_klx, addr);
	} else {
		snprintf(buf, sizeof(buf), timespec_fmt,
			 (long long) t.tv_sec,
			 zero_extend_signed_to_ull(t.tv_nsec));
	}

	return buf;
}

MPERS_PRINTER_DECL(void, print_timespec_utime_pair,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	timespec_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return;

	tprints("[");
	print_timespec_t_utime(&t[0]);
	tprints(", ");
	print_timespec_t_utime(&t[1]);
	tprints("]");
}

MPERS_PRINTER_DECL(void, print_itimerspec,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	timespec_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return;

	tprints("{it_interval=");
	print_timespec_t(&t[0]);
	tprints(", it_value=");
	print_timespec_t(&t[1]);
	tprints("}");
}
