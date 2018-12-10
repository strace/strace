/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(timeval_t)

typedef struct timeval timeval_t;

#include MPERS_DEFS

#include "xstring.h"

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

MPERS_PRINTER_DECL(bool, print_struct_timeval_data_size,
		   const void *arg, const size_t size)
{
	if (size < sizeof(timeval_t)) {
		tprints("?");
		return false;
	}

	print_timeval_t(arg);
	return true;
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
		xsprintf(buf, "%#" PRI_klx, addr);
	} else {
		xsprintf(buf, timeval_fmt,
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
		xsprintf(buf, "%#" PRI_klx, addr);
	} else {
		xsprintf(buf, timeval_fmt,
			 (long long) t.tv_sec,
			 zero_extend_signed_to_ull(t.tv_usec));
	}

	return buf;
}

#endif /* ALPHA */
