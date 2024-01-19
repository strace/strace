/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "xstring.h"

#ifndef TIMESPEC_NSEC
# define TIMESPEC_NSEC tv_nsec
#endif

#ifndef UTIME_NOW
# define UTIME_NOW ((1l << 30) - 1l)
#endif
#ifndef UTIME_OMIT
# define UTIME_OMIT ((1l << 30) - 2l)
#endif

#define TIMESPEC_TO_SEC_NSEC(t_)	\
	((long long) (t_)->tv_sec),	\
	 zero_extend_signed_to_ull((t_)->TIMESPEC_NSEC)

static const char timespec_fmt[] =
	"{tv_sec=%lld, " STRINGIFY_VAL(TIMESPEC_NSEC) "=%llu}";

static void
print_timespec_t(const TIMESPEC_T *t)
{
	tprint_struct_begin();
	PRINT_FIELD_D(*t, tv_sec);
	tprint_struct_next();
	tprints_field_name(STRINGIFY_VAL(TIMESPEC_NSEC));
	PRINT_VAL_U(t->TIMESPEC_NSEC);
	tprint_struct_end();
}

#if defined PRINT_TIMESPEC_DATA_SIZE || defined PRINT_TIMESPEC_ARRAY_DATA_SIZE
static void
print_unaligned_timespec_t(const void *arg)
{
	TIMESPEC_T t;
	memcpy(&t, arg, sizeof(t));
	print_timespec_t(&t);
}
#endif /* PRINT_TIMESPEC_DATA_SIZE || PRINT_TIMESPEC_ARRAY_DATA_SIZE */

#ifdef PRINT_TIMESPEC_DATA_SIZE
bool
PRINT_TIMESPEC_DATA_SIZE(const void *arg, const size_t size)
{
	if (size < sizeof(TIMESPEC_T)) {
		tprint_unavailable();
		return false;
	}

	print_unaligned_timespec_t(arg);
	return true;
}
#endif /* PRINT_TIMESPEC_DATA_SIZE */

#ifdef PRINT_TIMESPEC_ARRAY_DATA_SIZE
bool
PRINT_TIMESPEC_ARRAY_DATA_SIZE(const void *arg, const unsigned int nmemb,
			       const size_t size)
{
	if (nmemb > size / sizeof(TIMESPEC_T)) {
		tprint_unavailable();
		return false;
	}

	tprint_array_begin();

	for (unsigned int i = 0; i < nmemb; i++, arg += sizeof(TIMESPEC_T)) {
		if (i)
			tprint_array_next();
		print_unaligned_timespec_t(arg);
	}

	tprint_array_end();
	return true;
}
#endif /* PRINT_TIMESPEC_ARRAY_DATA_SIZE */

#ifdef PRINT_TIMESPEC
int
PRINT_TIMESPEC(struct tcb *const tcp, const kernel_ulong_t addr)
{
	TIMESPEC_T t;

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	print_timespec_t(&t);
	return 0;
}
#endif /* PRINT_TIMESPEC */

#ifdef SPRINT_TIMESPEC
const char *
SPRINT_TIMESPEC(struct tcb *const tcp, const kernel_ulong_t addr)
{
	TIMESPEC_T t;
	static char buf[sizeof(timespec_fmt) + 3 * sizeof(t)];

	if (!addr) {
		strcpy(buf, "NULL");
	} else if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
		   umove(tcp, addr, &t)) {
		xsprintf(buf, "%#" PRI_klx, addr);
	} else {
		xsprintf(buf, timespec_fmt, TIMESPEC_TO_SEC_NSEC(&t));
	}

	return buf;
}
#endif /* SPRINT_TIMESPEC */

#ifdef PRINT_TIMESPEC_UTIME_PAIR
static bool
print_timespec_t_utime(struct tcb *tcp, void *elem_buf, size_t elem_size,
		       void *data)
{
	const TIMESPEC_T *const t = elem_buf;
	switch (t->TIMESPEC_NSEC) {
	case UTIME_NOW:
	case UTIME_OMIT:
		if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
			print_timespec_t(t);
		if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
			break;

		(xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE
			? tprints_comment : tprints_string)(t->TIMESPEC_NSEC == UTIME_NOW
				? "UTIME_NOW" : "UTIME_OMIT");
		break;
	default:
		print_timespec_t(t);
		tprints_comment(sprinttime_nsec(TIMESPEC_TO_SEC_NSEC(t)));
		break;
	}
	return true;
}

int
PRINT_TIMESPEC_UTIME_PAIR(struct tcb *const tcp, const kernel_ulong_t addr)
{
	TIMESPEC_T t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	print_local_array(tcp, t, print_timespec_t_utime);
	return 0;
}
#endif /* PRINT_TIMESPEC_UTIME_PAIR */

#ifdef PRINT_ITIMERSPEC
int
PRINT_ITIMERSPEC(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct { TIMESPEC_T it_interval, it_value; } t;

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_PTR(t, it_interval, print_timespec_t);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(t, it_value, print_timespec_t);
	tprint_struct_end();
	return 0;
}
#endif /* PRINT_ITIMERSPEC */
