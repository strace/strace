/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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

#include DEF_MPERS_TYPE(timeval_t)

typedef struct timeval timeval_t;

#include MPERS_DEFS

static const char timeval_fmt[]  = "{tv_sec=%lld, tv_usec=%llu}";

static void
print_timeval_t(const timeval_t *t)
{
	tprintf(timeval_fmt, (long long) t->tv_sec,
		zero_extend_signed_to_ull(t->tv_usec));
}

static void
print_timeval_t_utime(const timeval_t *t)
{
	print_timeval_t(t);
	tprints_comment(sprinttime_usec(t->tv_sec,
		zero_extend_signed_to_ull(t->tv_usec)));
}

MPERS_PRINTER_DECL(void, print_struct_timeval, const void *arg)
{
	print_timeval_t(arg);
}

MPERS_PRINTER_DECL(void, print_timeval,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval_t t;

	if (umove_or_printaddr(tcp, addr, &t))
		return;

	print_timeval_t(&t);
}

MPERS_PRINTER_DECL(void, print_timeval_utimes,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return;

	tprints("[");
	print_timeval_t_utime(&t[0]);
	tprints(", ");
	print_timeval_t_utime(&t[1]);
	tprints("]");
}

MPERS_PRINTER_DECL(const char *, sprint_timeval,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval_t t;
	static char buf[sizeof(timeval_fmt) + 3 * sizeof(t)];

	if (!addr) {
		strcpy(buf, "NULL");
	} else if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
		   umove(tcp, addr, &t)) {
		snprintf(buf, sizeof(buf), "%#" PRI_klx, addr);
	} else {
		snprintf(buf, sizeof(buf), timeval_fmt,
			 (long long) t.tv_sec,
			 zero_extend_signed_to_ull(t.tv_usec));
	}

	return buf;
}

MPERS_PRINTER_DECL(void, print_itimerval,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return;

	tprints("{it_interval=");
	print_timeval_t(&t[0]);
	tprints(", it_value=");
	print_timeval_t(&t[1]);
	tprints("}");
}

#ifdef ALPHA

void
print_timeval32_t(const timeval32_t *t)
{
	tprintf(timeval_fmt, (long long) t->tv_sec,
		zero_extend_signed_to_ull(t->tv_usec));
}

static void
print_timeval32_t_utime(const timeval32_t *t)
{
	print_timeval32_t(t);
	tprints_comment(sprinttime_usec(t->tv_sec,
		zero_extend_signed_to_ull(t->tv_usec)));
}

void
print_timeval32(struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval32_t t;

	if (umove_or_printaddr(tcp, addr, &t))
		return;

	print_timeval32_t(&t);
}

void
print_timeval32_utimes(struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval32_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return;

	tprints("[");
	print_timeval32_t_utime(&t[0]);
	tprints(", ");
	print_timeval32_t_utime(&t[1]);
	tprints("]");
}

void
print_itimerval32(struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval32_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return;

	tprints("{it_interval=");
	print_timeval32_t(&t[0]);
	tprints(", it_value=");
	print_timeval32_t(&t[1]);
	tprints("}");
}

const char *
sprint_timeval32(struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval32_t t;
	static char buf[sizeof(timeval_fmt) + 3 * sizeof(t)];

	if (!addr) {
		strcpy(buf, "NULL");
	} else if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
		   umove(tcp, addr, &t)) {
		snprintf(buf, sizeof(buf), "%#" PRI_klx, addr);
	} else {
		snprintf(buf, sizeof(buf), timeval_fmt,
			 (long long) t.tv_sec,
			 zero_extend_signed_to_ull(t.tv_usec));
	}

	return buf;
}

#endif /* ALPHA */
