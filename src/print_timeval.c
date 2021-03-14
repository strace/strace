/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
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
	tprint_struct_begin();
	PRINT_FIELD_D(*t, tv_sec);
	tprint_struct_next();
	PRINT_FIELD_U(*t, tv_usec);
	tprint_struct_end();
}

static bool
print_timeval_t_utime(struct tcb *tcp, void *elem_buf, size_t elem_size,
		      void *data)
{
	const timeval_t *const t = elem_buf;
	print_timeval_t(t);
	tprints_comment(sprinttime_usec(t->tv_sec,
		zero_extend_signed_to_ull(t->tv_usec)));
	return true;
}

MPERS_PRINTER_DECL(void, print_struct_timeval, const void *arg)
{
	print_timeval_t(arg);
}

MPERS_PRINTER_DECL(bool, print_struct_timeval_data_size,
		   const void *arg, const size_t size)
{
	if (size < sizeof(timeval_t)) {
		tprint_unavailable();
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

	print_local_array(tcp, t, print_timeval_t_utime);
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
	struct { timeval_t it_interval, it_value; } t;

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_PTR(t, it_interval, print_timeval_t);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(t, it_value, print_timeval_t);
	tprint_struct_end();
	return 0;
}

#ifdef ALPHA

void
print_timeval32_t(const timeval32_t *t)
{
	tprint_struct_begin();
	PRINT_FIELD_D(*t, tv_sec);
	tprint_struct_next();
	PRINT_FIELD_U(*t, tv_usec);
	tprint_struct_end();
}

static bool
print_timeval32_t_utime(struct tcb *tcp, void *elem_buf, size_t elem_size,
			void *data)
{
	const timeval32_t *const t = elem_buf;
	print_timeval32_t(t);
	tprints_comment(sprinttime_usec(t->tv_sec,
		zero_extend_signed_to_ull(t->tv_usec)));
	return true;
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

	print_local_array(tcp, t, print_timeval32_t_utime);
	return 0;
}

int
print_itimerval32(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct { timeval32_t it_interval, it_value; } t;

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_PTR(t, it_interval, print_timeval32_t);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(t, it_value, print_timeval32_t);
	tprint_struct_end();
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
