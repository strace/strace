/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2006 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2006-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

/* Per-syscall stats structure */
struct call_counts {
	/* time may be total latency or system time */
	struct timespec time;
	unsigned int calls, errors;
};

static struct call_counts *countv[SUPPORTED_PERSONALITIES];
#define counts (countv[current_personality])

static const struct timespec zero_ts;

static struct timespec overhead;

void
count_syscall(struct tcb *tcp, const struct timespec *syscall_exiting_ts)
{
	if (!scno_in_range(tcp->scno))
		return;

	if (!counts)
		counts = xcalloc(nsyscalls, sizeof(*counts));
	struct call_counts *cc = &counts[tcp->scno];

	cc->calls++;
	if (syserror(tcp))
		cc->errors++;

	struct timespec wts;
	if (count_wallclock) {
		/* wall clock time spent while in syscall */
		ts_sub(&wts, syscall_exiting_ts, &tcp->etime);
	} else {
		/* system CPU time spent while in syscall */
		ts_sub(&wts, &tcp->stime, &tcp->ltime);
	}

	ts_sub(&wts, &wts, &overhead);
	ts_add(&cc->time, &cc->time, ts_max(&wts, &zero_ts));
}

static int
time_cmp(const void *a, const void *b)
{
	const unsigned int *a_int = a;
	const unsigned int *b_int = b;
	return -ts_cmp(&counts[*a_int].time, &counts[*b_int].time);
}

static int
syscall_cmp(const void *a, const void *b)
{
	const unsigned int *a_int = a;
	const unsigned int *b_int = b;
	const char *a_name = sysent[*a_int].sys_name;
	const char *b_name = sysent[*b_int].sys_name;
	return strcmp(a_name ? a_name : "", b_name ? b_name : "");
}

static int
count_cmp(const void *a, const void *b)
{
	const unsigned int *a_int = a;
	const unsigned int *b_int = b;
	unsigned int m = counts[*a_int].calls;
	unsigned int n = counts[*b_int].calls;

	return (m < n) ? 1 : (m > n) ? -1 : 0;
}

static int
error_cmp(const void *a, const void *b)
{
	const unsigned int *a_int = a;
	const unsigned int *b_int = b;
	unsigned int m = counts[*a_int].errors;
	unsigned int n = counts[*b_int].errors;

	return (m < n) ? 1 : (m > n) ? -1 : 0;
}

static int (*sortfun)(const void *, const void *);

void
set_sortby(const char *sortby)
{
	static const struct {
		int (*fn)(const void *, const void *);
		const char *name;
	} sort_fns[] = {
		{ time_cmp,	"time" },
		{ time_cmp,	"time_total" },
		{ time_cmp,	"total_time" },
		{ count_cmp,	"calls" },
		{ count_cmp,	"count" },
		{ error_cmp,	"error" },
		{ error_cmp,	"errors" },
		{ syscall_cmp,	"name" },
		{ syscall_cmp,	"syscall" },
		{ syscall_cmp,	"syscall_name" },
		{ NULL,		"none" },
		{ NULL,		"nothing" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(sort_fns); ++i) {
		if (!strcmp(sort_fns[i].name, sortby)) {
			sortfun = sort_fns[i].fn;
			return;
		}
	}

	error_msg_and_help("invalid sortby: '%s'", sortby);
}

int
set_overhead(const char *str)
{
	return parse_ts(str, &overhead);
}

static void
call_summary_pers(FILE *outf)
{
	static const char dashes[]  = "----------------";
	static const char header[]  = "%6.6s %11.11s %11.11s %9.9s %9.9s %s\n";
	static const char data[]    = "%6.2f %11.6f %11lu %9u %9.u %s\n";
	static const char summary[] = "%6.6s %11.6f %11.11s %9u %9.u %s\n";

	unsigned int i;
	unsigned int call_cum, error_cum;
	struct timespec tv_cum, dtv;
	double  float_tv_cum;
	double  percent;
	unsigned int *sorted_count;

	fprintf(outf, header,
		"% time", "seconds", "usecs/call",
		"calls", "errors", "syscall");
	fprintf(outf, header, dashes, dashes, dashes, dashes, dashes, dashes);

	sorted_count = xcalloc(sizeof(sorted_count[0]), nsyscalls);
	call_cum = error_cum = tv_cum.tv_sec = tv_cum.tv_nsec = 0;
	for (i = 0; i < nsyscalls; i++) {
		sorted_count[i] = i;
		if (counts == NULL || counts[i].calls == 0)
			continue;
		call_cum += counts[i].calls;
		error_cum += counts[i].errors;
		ts_add(&tv_cum, &tv_cum, &counts[i].time);
	}
	float_tv_cum = ts_float(&tv_cum);
	if (counts) {
		if (sortfun)
			qsort((void *) sorted_count, nsyscalls,
			      sizeof(sorted_count[0]), sortfun);
		for (i = 0; i < nsyscalls; i++) {
			double float_syscall_time;
			unsigned int idx = sorted_count[i];
			struct call_counts *cc = &counts[idx];
			if (cc->calls == 0)
				continue;
			ts_div(&dtv, &cc->time, cc->calls);
			float_syscall_time = ts_float(&cc->time);
			percent = (100.0 * float_syscall_time);
			if (percent != 0.0)
				   percent /= float_tv_cum;
			/* else: float_tv_cum can be 0.0 too and we get 0/0 = NAN */
			fprintf(outf, data,
				percent, float_syscall_time,
				(long) (1000000 * dtv.tv_sec + dtv.tv_nsec / 1000),
				cc->calls, cc->errors, sysent[idx].sys_name);
		}
	}
	free(sorted_count);

	fprintf(outf, header, dashes, dashes, dashes, dashes, dashes, dashes);
	fprintf(outf, summary,
		"100.00", float_tv_cum, "",
		call_cum, error_cum, "total");
}

void
call_summary(FILE *outf)
{
	unsigned int i, old_pers = current_personality;

	for (i = 0; i < SUPPORTED_PERSONALITIES; ++i) {
		if (!countv[i])
			continue;

		if (current_personality != i)
			set_personality(i);
		if (i)
			fprintf(outf,
				"System call usage summary for %s mode:\n",
				personality_names[i]);
		call_summary_pers(outf);
	}

	if (old_pers != current_personality)
		set_personality(old_pers);
}
