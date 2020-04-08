/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(timeval_t)

#include "kernel_timeval.h"

typedef kernel_old_timeval_t timeval_t;

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

MPERS_PRINTER_DECL(int, print_timeval,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval_t t;

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	print_timeval_t(&t);
	return 0;
}

MPERS_PRINTER_DECL(int, print_timeval_utimes,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	tprints("[");
	print_timeval_t_utime(&t[0]);
	tprints(", ");
	print_timeval_t_utime(&t[1]);
	tprints("]");
	return 0;
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

MPERS_PRINTER_DECL(int, print_itimerval,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	tprints("{it_interval=");
	print_timeval_t(&t[0]);
	tprints(", it_value=");
	print_timeval_t(&t[1]);
	tprints("}");
	return 0;
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

int
print_timeval32(struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval32_t t;

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	print_timeval32_t(&t);
	return 0;
}

int
print_timeval32_utimes(struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval32_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	tprints("[");
	print_timeval32_t_utime(&t[0]);
	tprints(", ");
	print_timeval32_t_utime(&t[1]);
	tprints("]");
	return 0;
}

int
print_itimerval32(struct tcb *const tcp, const kernel_ulong_t addr)
{
	timeval32_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	tprints("{it_interval=");
	print_timeval32_t(&t[0]);
	tprints(", it_value=");
	print_timeval32_t(&t[1]);
	tprints("}");
	return 0;
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
