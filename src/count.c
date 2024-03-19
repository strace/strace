/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2006 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2006-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <stdarg.h>

/* Per-syscall stats structure */
struct call_counts {
	/* time may be total latency or system time */
	struct timespec time;
	struct timespec time_min;
	struct timespec time_max;
	struct timespec time_avg;
	uint64_t calls, errors;
};

static struct call_counts *countv[SUPPORTED_PERSONALITIES];
#define counts (countv[current_personality])

static const struct timespec zero_ts;
static const struct timespec max_ts = {
	(time_t) (long long) (zero_extend_signed_to_ull((time_t) -1ULL) >> 1),
	999999999 };

static struct timespec overhead;


enum count_summary_columns {
	CSC_NONE,
	CSC_TIME_100S,
	CSC_TIME_TOTAL,
	CSC_TIME_MIN,
	CSC_TIME_MAX,
	CSC_TIME_AVG,
	CSC_CALLS,
	CSC_ERRORS,
	CSC_SC_NAME,

	CSC_MAX,
};

static uint8_t columns[CSC_MAX] = {
	CSC_TIME_100S,
	CSC_TIME_TOTAL,
	CSC_TIME_AVG,
	CSC_CALLS,
	CSC_ERRORS,
	CSC_SC_NAME,
};

static const struct {
	const char *name;
	uint8_t     column;
} column_aliases[] = {
	{ "time",         CSC_TIME_100S  },
	{ "time_percent", CSC_TIME_100S  },
	{ "time-percent", CSC_TIME_100S  },
	{ "time_total",   CSC_TIME_TOTAL },
	{ "time-total",   CSC_TIME_TOTAL },
	{ "total_time",   CSC_TIME_TOTAL },
	{ "total-time",   CSC_TIME_TOTAL },
	{ "min_time",     CSC_TIME_MIN   },
	{ "min-time",     CSC_TIME_MIN   },
	{ "shortest",     CSC_TIME_MIN   },
	{ "time_min",     CSC_TIME_MIN   },
	{ "time-min",     CSC_TIME_MIN   },
	{ "longest" ,     CSC_TIME_MAX   },
	{ "max_time",     CSC_TIME_MAX   },
	{ "max-time",     CSC_TIME_MAX   },
	{ "time_max",     CSC_TIME_MAX   },
	{ "time-max",     CSC_TIME_MAX   },
	{ "avg_time",     CSC_TIME_AVG   },
	{ "avg-time",     CSC_TIME_AVG   },
	{ "time_avg",     CSC_TIME_AVG   },
	{ "time-avg",     CSC_TIME_AVG   },
	{ "calls",        CSC_CALLS      },
	{ "count",        CSC_CALLS      },
	{ "error",        CSC_ERRORS     },
	{ "errors",       CSC_ERRORS     },
	{ "name",         CSC_SC_NAME    },
	{ "syscall",      CSC_SC_NAME    },
	{ "syscall_name", CSC_SC_NAME    },
	{ "syscall-name", CSC_SC_NAME    },
	{ "none",         CSC_NONE       },
	{ "nothing",      CSC_NONE       },
};

void
count_syscall(struct tcb *tcp, const struct timespec *syscall_exiting_ts)
{
	if (!scno_in_range(tcp->scno))
		return;

	if (!counts) {
		counts = xcalloc(nsyscalls, sizeof(*counts));

		for (size_t i = 0; i < nsyscalls; i++)
			counts[i].time_min = max_ts;
	}
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

	const struct timespec *wts_nonneg = ts_max(&wts, &zero_ts);

	ts_add(&cc->time, &cc->time, wts_nonneg);
	cc->time_min = *ts_min(&cc->time_min, wts_nonneg);
	cc->time_max = *ts_max(&cc->time_max, wts_nonneg);
}

static int
time_cmp(const void *a, const void *b)
{
	const unsigned int *a_int = a;
	const unsigned int *b_int = b;
	return -ts_cmp(&counts[*a_int].time, &counts[*b_int].time);
}

static int
min_time_cmp(const void *a, const void *b)
{
	return -ts_cmp(&counts[*((unsigned int *) a)].time_min,
		       &counts[*((unsigned int *) b)].time_min);
}

static int
max_time_cmp(const void *a, const void *b)
{
	return -ts_cmp(&counts[*((unsigned int *) a)].time_max,
		       &counts[*((unsigned int *) b)].time_max);
}

static int
avg_time_cmp(const void *a, const void *b)
{
	return -ts_cmp(&counts[*((unsigned int *) a)].time_avg,
		       &counts[*((unsigned int *) b)].time_avg);
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

typedef int (*sort_func)(const void *, const void *);
static sort_func sortfun;

void
set_sortby(const char *sortby)
{
	static const sort_func sort_fns[CSC_MAX] = {
		[CSC_TIME_100S]  = time_cmp,
		[CSC_TIME_TOTAL] = time_cmp,
		[CSC_TIME_MIN]   = min_time_cmp,
		[CSC_TIME_MAX]   = max_time_cmp,
		[CSC_TIME_AVG]   = avg_time_cmp,
		[CSC_CALLS]      = count_cmp,
		[CSC_ERRORS]     = error_cmp,
		[CSC_SC_NAME]    = syscall_cmp,
	};

	for (size_t i = 0; i < ARRAY_SIZE(column_aliases); ++i) {
		if (!strcmp(column_aliases[i].name, sortby)) {
			sortfun = sort_fns[column_aliases[i].column];
			return;
		}
	}

	error_msg_and_help("invalid sortby: '%s'", sortby);
}

void
set_count_summary_columns(const char *s)
{
	uint8_t visible[CSC_MAX] = { 0 };
	const char *prev = s;
	size_t cur = 0;

	memset(columns, 0, sizeof(columns));

	for (;;) {
		bool found = false;
		const char *pos = strchr(prev, ',');
		size_t len = pos ? (size_t) (pos - prev) : strlen(prev);

		for (size_t i = 0; i < ARRAY_SIZE(column_aliases); i++) {
			if (strncmp(column_aliases[i].name, prev, len) ||
			    column_aliases[i].name[len])
				continue;
			if (column_aliases[i].column == CSC_NONE ||
			    column_aliases[i].column >= CSC_MAX)
				continue;

			if (visible[column_aliases[i].column])
				error_msg_and_help("call summary column "
						   "has been provided more "
						   "than once: '%s' (-U option "
						   "residual: '%s')",
						   column_aliases[i].name,
						   prev);

			columns[cur++] = column_aliases[i].column;
			visible[column_aliases[i].column] = 1;
			found = true;

			break;
		}

		if (!found)
			error_msg_and_help("unknown column name: '%.*s'",
					   (int) MIN(len, INT_MAX), prev);

		if (!pos)
			break;

		prev = pos + 1;
	}

	/*
	 * Always enable syscall name column, as without it table is meaningless
	 */
	if (!visible[CSC_SC_NAME])
		columns[cur++] = CSC_SC_NAME;
}

int
set_overhead(const char *str)
{
	return parse_ts(str, &overhead);
}

static size_t ATTRIBUTE_FORMAT((printf, 1, 2))
num_chars(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	int ret = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	return (unsigned int) MAX(ret, 0);
}

static void
call_summary_pers(FILE *outf)
{
	unsigned int *indices;
	size_t last_column = 0;

	struct timespec tv_cum = zero_ts;
	const struct timespec *tv_min = &max_ts;
	const struct timespec *tv_min_max = &zero_ts;
	const struct timespec *tv_max = &zero_ts;
	const struct timespec *tv_avg_max = &zero_ts;
	uint64_t call_cum = 0;
	uint64_t error_cum = 0;

	double float_tv_cum;
	double percent;

	size_t sc_name_max = 0;


	/* sort, calculate statistics */
	indices = xcalloc(nsyscalls, sizeof(indices[0]));
	for (size_t i = 0; i < nsyscalls; ++i) {
		indices[i] = i;
		if (counts[i].calls == 0)
			continue;

		ts_add(&tv_cum, &tv_cum, &counts[i].time);
		tv_min = ts_min(tv_min, &counts[i].time_min);
		tv_min_max = ts_max(tv_min_max, &counts[i].time_min);
		tv_max = ts_max(tv_max, &counts[i].time_max);
		call_cum += counts[i].calls;
		error_cum += counts[i].errors;

		ts_div(&counts[i].time_avg, &counts[i].time, counts[i].calls);
		tv_avg_max = ts_max(tv_avg_max, &counts[i].time_avg);

		sc_name_max = MAX(sc_name_max, strlen(sysent[i].sys_name));
	}
	float_tv_cum = ts_float(&tv_cum);

	if (sortfun)
		qsort((void *) indices, nsyscalls, sizeof(indices[0]), sortfun);

	enum column_flags {
		CF_L = 1 << 0, /* Left-aligned column */
	};
	static const struct {
		const char *s;
		size_t sz;
		const char *fmt;
		const char *last_fmt;
		uint32_t flags;
	} cdesc[] = {
		[CSC_TIME_100S]  = { ARRSZ_PAIR("% time") - 1,   "%1$*2$.2f" },
		[CSC_TIME_MIN]   = { ARRSZ_PAIR("shortest") - 1, "%1$*2$.6f" },
		[CSC_TIME_MAX]   = { ARRSZ_PAIR("longest") - 1,  "%1$*2$.6f" },
		/* Historical field sizes are preserved */
		[CSC_TIME_TOTAL] = { "seconds",    11, "%1$*2$.6f" },
		[CSC_TIME_AVG]   = { "usecs/call", 11, "%1$*2$" PRIu64 },
		[CSC_CALLS]      = { "calls",       9, "%1$*2$" PRIu64 },
		[CSC_ERRORS]     = { "errors",      9, "%1$*2$.0" PRIu64 },
		[CSC_SC_NAME]    = { "syscall",    16, "%1$-*2$s", "%1$s", CF_L },
	};

	/* calculate column widths */
#define W_(c_, v_) [c_] = MAX(cdesc[c_].sz, (v_))
	unsigned int cwidths[CSC_MAX] = {
		W_(CSC_TIME_100S,  sizeof("100.00") - 1),
		W_(CSC_TIME_TOTAL, num_chars("%.6f", float_tv_cum)),
		W_(CSC_TIME_MIN,   num_chars("%" PRId64 ".000000",
					     (int64_t) tv_min_max->tv_sec)),
		W_(CSC_TIME_MAX,   num_chars("%" PRId64 ".000000",
					     (int64_t) tv_max->tv_sec)),
		W_(CSC_TIME_AVG,   num_chars("%" PRId64 ,
					     (uint64_t) (ts_float(tv_avg_max)
							 * 1e6))),
		W_(CSC_CALLS,      num_chars("%" PRIu64, call_cum)),
		W_(CSC_ERRORS,     num_chars("%" PRIu64, error_cum)),
		W_(CSC_SC_NAME,    sc_name_max + 1),
	};
#undef W_

	/* find the last column */
	for (size_t i = 0; i < ARRAY_SIZE(columns) && columns[i]; ++i)
		last_column = i;

	/* header */
	for (size_t i = 0; i <= last_column; ++i) {
		const char *fmt = cdesc[columns[i]].flags & CF_L
				  ? (i == last_column ? "%1$s" : "%1$-*2$s")
				  : "%1$*2$s";
		if (i)
			fputc(' ', outf);
		fprintf(outf, fmt, cdesc[columns[i]].s, cwidths[columns[i]]);
	}
	fputc('\n', outf);

	/* divider */
	for (size_t i = 0; i <= last_column; ++i) {
		if (i)
			fputc(' ', outf);

		for (size_t j = 0; j < cwidths[columns[i]]; ++j)
			fputc('-', outf);
	}
	fputc('\n', outf);

	/* cache column formats */
#define FC_(c_) \
	case (c_): \
		column_fmts[i] = (i == last_column) && cdesc[c].last_fmt \
				 ? cdesc[c].last_fmt : cdesc[c].fmt; \
		break
#define PC_(c_, val_) \
	case (c_): \
		fprintf(outf, column_fmts[i], (val_), cwidths[c]); \
		break

	const char *column_fmts[last_column + 1];
	for (size_t i = 0; i <= last_column; ++i) {
		const size_t c = columns[i];

		switch (c) {
		FC_(CSC_TIME_100S);
		FC_(CSC_TIME_TOTAL);
		FC_(CSC_TIME_MIN);
		FC_(CSC_TIME_MAX);
		FC_(CSC_TIME_AVG);
		FC_(CSC_CALLS);
		FC_(CSC_ERRORS);
		FC_(CSC_SC_NAME);
		}
	}

	/* data output */
	for (size_t j = 0; j < nsyscalls; ++j) {
		unsigned int idx = indices[j];
		struct call_counts *cc = &counts[idx];
		double float_syscall_time;

		if (cc->calls == 0)
			continue;

		float_syscall_time = ts_float(&cc->time);
		percent = (100.0 * float_syscall_time);
		/* else: float_tv_cum can be 0.0 too and we get 0/0 = NAN */
		if (percent != 0.0)
			   percent /= float_tv_cum;

		for (size_t i = 0; i <= last_column; ++i) {
			const size_t c = columns[i];
			if (i)
				fputc(' ', outf);

			switch (c) {
			PC_(CSC_TIME_100S,  percent);
			PC_(CSC_TIME_TOTAL, float_syscall_time);
			PC_(CSC_TIME_MIN,   ts_float(&cc->time_min));
			PC_(CSC_TIME_MAX,   ts_float(&cc->time_max));
			PC_(CSC_TIME_AVG,
			    (uint64_t) (ts_float(&cc->time_avg) * 1e6));
			PC_(CSC_CALLS,      cc->calls);
			PC_(CSC_ERRORS,     cc->errors);
			PC_(CSC_SC_NAME,    sysent[idx].sys_name);
			}
		}

		fputc('\n', outf);
	}

	free(indices);

	/* footer */
	for (size_t i = 0; i <= last_column; ++i) {
		if (i)
			fputc(' ', outf);

		for (size_t j = 0; j < cwidths[columns[i]]; ++j)
			fputc('-', outf);
	}
	fputc('\n', outf);

	/* totals */
	for (size_t i = 0; i <= last_column; ++i) {
		const size_t c = columns[i];
		if (i)
			fputc(' ', outf);

		switch (c) {
		PC_(CSC_TIME_100S, 100.0);
		PC_(CSC_TIME_TOTAL, float_tv_cum);
		PC_(CSC_TIME_MIN, ts_float(tv_min));
		PC_(CSC_TIME_MAX, ts_float(tv_max));
		PC_(CSC_TIME_AVG, (uint64_t) (float_tv_cum / call_cum * 1e6));
		PC_(CSC_CALLS, call_cum);
		PC_(CSC_ERRORS, error_cum);
		PC_(CSC_SC_NAME, "total");
		}
	}
	fputc('\n', outf);

#undef PC_
#undef FC_
}

void
call_summary(FILE *outf)
{
	const unsigned int old_pers = current_personality;

	for (unsigned int i = 0; i < SUPPORTED_PERSONALITIES; ++i) {
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
