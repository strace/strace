/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
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
print_sec_nsec(long long sec, unsigned long long nsec)
{
	tprintf(timespec_fmt, sec, nsec);
}

static void
print_timespec_t(const TIMESPEC_T *t)
{
	print_sec_nsec(TIMESPEC_TO_SEC_NSEC(t));
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
		tprints("?");
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
		tprints("?");
		return false;
	}

	tprints("[");

	for (unsigned int i = 0; i < nmemb; i++, arg += sizeof(TIMESPEC_T)) {
		if (i)
			tprints(", ");
		print_unaligned_timespec_t(arg);
	}

	tprints("]");
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
static void
print_timespec_t_utime(const TIMESPEC_T *t)
{
	switch (t->TIMESPEC_NSEC) {
	case UTIME_NOW:
	case UTIME_OMIT:
		if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
			print_timespec_t(t);
		if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
			break;

		(xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE
			? tprints_comment : tprints)(t->TIMESPEC_NSEC == UTIME_NOW
				? "UTIME_NOW" : "UTIME_OMIT");
		break;
	default:
		print_timespec_t(t);
		tprints_comment(sprinttime_nsec(TIMESPEC_TO_SEC_NSEC(t)));
		break;
	}
}

int
PRINT_TIMESPEC_UTIME_PAIR(struct tcb *const tcp, const kernel_ulong_t addr)
{
	TIMESPEC_T t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	tprints("[");
	print_timespec_t_utime(&t[0]);
	tprints(", ");
	print_timespec_t_utime(&t[1]);
	tprints("]");
	return 0;
}
#endif /* PRINT_TIMESPEC_UTIME_PAIR */

#ifdef PRINT_ITIMERSPEC
int
PRINT_ITIMERSPEC(struct tcb *const tcp, const kernel_ulong_t addr)
{
	TIMESPEC_T t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return -1;

	tprints("{it_interval=");
	print_timespec_t(&t[0]);
	tprints(", it_value=");
	print_timespec_t(&t[1]);
	tprints("}");
	return 0;
}
#endif /* PRINT_ITIMERSPEC */
