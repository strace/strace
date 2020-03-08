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
	CSC_TIME_100S, /* type: percent; format: percent, value; precision=2 */
	CSC_TIME_TOTAL, /* type: time; format: s, ms, us, ns, s.ms, s.us, s.ns */
	CSC_TIME_MIN, /* type: time */
	CSC_TIME_MAX, /* tyoe: time */
	CSC_TIME_AVG, /* type: time */
	CSC_CALLS, /* type: number */
	CSC_ERRORS, /* type: number, print-zeroes=yes|no */
	CSC_SC_NAME, /* type scname, format: name, number, name (number) */

	CSC_MAX,
};

enum count_summary_column_type {
	CSCT_NONE,
	CSCT_PERCENT,
	CSCT_TIME,
	CSCT_NUMBER,
	CSCT_SC_NAME,

	CSCT_COUNT
};

union count_summary_column_val {
	double percent;
	struct timespec t;
	uint64_t num;
	struct {
		const char *sc_name;
		size_t sc_num;
	} se;
};

enum { CSCT_SHIFT = 3 };

#define COLUMN_TYPE(c_) ((c_) >> CSCT_SHIFT)

enum count_summary_column_formats {
	CSCT_NONE_NONE       = CSCT_NONE << CSCT_SHIFT,

	CSCT_PERCENT_PERCENT = CSCT_PERCENT << CSCT_SHIFT,
	CSCT_PERCENT_PART,

	CSCT_TIME_S          = CSCT_TIME << CSCT_SHIFT,
	CSCT_TIME_MS,
	CSCT_TIME_US,
	CSCT_TIME_NS,
	CSCT_TIME_S_MS,
	CSCT_TIME_S_US,
	CSCT_TIME_S_NS,

	CSCT_NUMBER_NUMBER   = CSCT_NUMBER << CSCT_SHIFT,

	CSCT_SC_NAME_NAME    = CSCT_SC_NAME << CSCT_SHIFT,
	CSCT_SC_NAME_NUMBER,
	CSCT_SC_NAME_NAME_NUMBER,
};

static const struct {
	const char *name;
	size_t name_len;
	enum count_summary_column_formats format;
} format_names[] = {
	{ ARRSZ_PAIR("percent"), CSCT_PERCENT_PERCENT },
	{ ARRSZ_PAIR("part"   ), CSCT_PERCENT_PART },
	{ ARRSZ_PAIR("s"      ), CSCT_TIME_S },
	{ ARRSZ_PAIR("ms"     ), CSCT_TIME_MS },
	{ ARRSZ_PAIR("us"     ), CSCT_TIME_US },
	{ ARRSZ_PAIR("ns"     ), CSCT_TIME_NS },
	{ ARRSZ_PAIR("s.ms"   ), CSCT_TIME_S_MS },
	{ ARRSZ_PAIR("s.us"   ), CSCT_TIME_S_US },
	{ ARRSZ_PAIR("s.ns"   ), CSCT_TIME_S_NS },
	{ ARRSZ_PAIR("name"   ), CSCT_SC_NAME_NAME },
	{ ARRSZ_PAIR("number" ), CSCT_SC_NAME_NUMBER },
	{ ARRSZ_PAIR("both"   ), CSCT_SC_NAME_NAME_NUMBER },
};

struct csct_config {
	enum count_summary_columns column;
	enum count_summary_column_formats format;
	uint8_t precision;    /* 0..20, CSCT_PERCENT */
	uint8_t print_zeroes; /* 0..1, CSCT_NUMBER */
};

static const struct csct_config column_types[CSC_MAX] = {
	[CSC_NONE]       = { CSC_NONE,	     CSCT_NONE },
	[CSC_TIME_100S]  = { CSC_TIME_100S,  CSCT_PERCENT_PERCENT,
				.precision = 2 },
	[CSC_TIME_TOTAL] = { CSC_TIME_TOTAL, CSCT_TIME_S_US },
	[CSC_TIME_MIN]   = { CSC_TIME_MIN,   CSCT_TIME_S_US },
	[CSC_TIME_MAX]   = { CSC_TIME_MAX,   CSCT_TIME_S_US },
	[CSC_TIME_AVG]   = { CSC_TIME_AVG,   CSCT_TIME_US },
	[CSC_CALLS]      = { CSC_CALLS,	     CSCT_NUMBER_NUMBER,
				.print_zeroes = true },
	[CSC_ERRORS]     = { CSC_ERRORS,     CSCT_NUMBER_NUMBER,
				.print_zeroes = false, },
	[CSC_SC_NAME]    = { CSC_SC_NAME,    CSCT_SC_NAME_NAME },
};

struct csct_config columns[CSC_MAX];

static const struct {
	const char *name;
	uint8_t     column;
} column_aliases[] = {
	{ "time",         CSC_TIME_100S  },
	{ "time_percent", CSC_TIME_100S  },
	{ "time-percent", CSC_TIME_100S  },
	{ "seconds",      CSC_TIME_TOTAL },
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
set_default_count_summary_columns(void)
{
	columns[0] = column_types[CSC_TIME_100S];
	columns[1] = column_types[CSC_TIME_TOTAL];
	columns[2] = column_types[CSC_TIME_AVG];
	columns[3] = column_types[CSC_CALLS];
	columns[4] = column_types[CSC_ERRORS];
	columns[5] = column_types[CSC_SC_NAME];
}

void
set_count_summary_columns(const char *s)
{
	uint8_t visible[CSC_MAX] = { 0 };
	const char *pos = s;
	const char *prev = s;
	size_t cur = 0;
	size_t len;
	const char *opt_pos;
	size_t opt_len;
	bool found;

	enum {
		TOKEN_FORMAT,
		TOKEN_PRECISION,
		TOKEN_ZEROES,
	} token_type;

	memset(columns, 0, sizeof(columns));

next_column:
	found = false;
	pos = strchr(prev, ',');
	len = pos ? (size_t) (pos - prev) : strlen(prev);

	opt_pos = strpbrk(prev, ",:");
	opt_len = opt_pos ? (size_t) (opt_pos - prev) : len;

	for (size_t i = 0; i < ARRAY_SIZE(column_aliases); i++) {
		if (strncmp(column_aliases[i].name, prev, opt_len) ||
		    column_aliases[i].name[opt_len])
			continue;
		if (column_aliases[i].column == CSC_NONE ||
		    column_aliases[i].column >= CSC_MAX)
			continue;

		if (visible[column_aliases[i].column])
			error_msg_and_help("call summary column has been "
					   "provided more than once: '%s' (-U "
					   "option residual: '%s')",
					   column_aliases[i].name,
					   prev);

		columns[cur] = column_types[column_aliases[i].column];
		visible[column_aliases[i].column] = 1;
		found = true;
		token_type = TOKEN_FORMAT;

		break;
	}

	if (!found)
		error_msg_and_help("unknown column name: '%.*s'",
				   (int) MIN(len, INT_MAX), prev);

	if (opt_pos == pos)
		goto end_opt;

	prev = opt_pos + 1;
	len -= opt_len + 1;
	opt_pos = strpbrk(prev, ",:");
	if (opt_pos >= pos)
		opt_pos = pos;
	opt_len = opt_pos ? (size_t) (opt_pos - prev) : len;

	while (true) {
		static const char format_pfx[] = "format=";
		static const char precision_pfx[]
			= "precision=";
		static const char zeroes_pfx[] = "show_zeroes=";

		if (opt_len >= sizeof(format_pfx) &&
		    !strncasecmp(prev, format_pfx, sizeof(format_pfx) - 1)) {
			prev += sizeof(format_pfx) - 1;
			opt_len -= sizeof(format_pfx) - 1;
			token_type = TOKEN_FORMAT;
		} else if (opt_len >= sizeof(precision_pfx) &&
			   COLUMN_TYPE(columns[cur].format) == CSCT_PERCENT &&
			   !strncasecmp(prev, precision_pfx,
					sizeof(precision_pfx) - 1)) {
			prev += sizeof(precision_pfx) - 1;
			opt_len -= sizeof(precision_pfx) - 1;
			token_type = TOKEN_PRECISION;
		} else if (opt_len >= sizeof(zeroes_pfx) &&
			   COLUMN_TYPE(columns[cur].format) == CSCT_NUMBER &&
			   !strncasecmp(prev, zeroes_pfx,
					sizeof(zeroes_pfx) - 1)) {
			prev += sizeof(zeroes_pfx) - 1;
			opt_len -= sizeof(zeroes_pfx) - 1;
			token_type = TOKEN_ZEROES;
		}

		switch (token_type) {
		case TOKEN_FORMAT: {
			bool format_found = false;

			for (size_t j = 0;
			     j < ARRAY_SIZE(format_names); j++) {
				if (!strncasecmp(prev, format_names[j].name,
						 format_names[j].name_len - 1)
				    && opt_len == format_names[j].name_len - 1
				    && COLUMN_TYPE(format_names[j].format)
				       == COLUMN_TYPE(columns[cur].format)) {
					columns[cur].format =
						format_names[j].format;
					format_found = true;

					break;
				}
			}

			if (!format_found) {
				error_msg_and_help("Unknown column format: "
						   "'%.*s'",
						   (int) MIN(opt_len, INT_MAX),
						   prev);
			}

			break;
		}
		case TOKEN_PRECISION: {
			long long val = string_to_uint_ex(prev, NULL, 20,
							  ":,");

			if (val < 0) {
				error_msg_and_help("Invalid precision: '%.*s'",
						   (int) MIN(opt_len, INT_MAX),
						   prev);
			}

			columns[cur].precision = val;

			break;
		}
		case TOKEN_ZEROES: {
			int val = string_to_bool(prev, ":,");

			if (val < 0) {
				error_msg_and_help("Invalid bool value: '%.*s'",
						   (int) MIN(opt_len, INT_MAX),
						   prev);
			}

			columns[cur].print_zeroes = val;

			break;
		}
		}

		if (!opt_pos || *opt_pos == ',')
			break;

		prev = opt_pos + 1;
		len -= opt_len + 1;
		opt_pos = strpbrk(prev, ",:");
		if (opt_pos >= pos)
			opt_pos = pos;
		opt_len = opt_pos ? (size_t) (opt_pos - prev) : len;
	}

end_opt:
	cur++;

	if (!pos)
		goto end;

	prev = pos + 1;

	goto next_column;

end:
	/*
	 * Always enable syscall name column, as without it table is meaningless
	 */
	if (!visible[CSC_SC_NAME])
		columns[cur++] = column_types[CSC_SC_NAME];
}

int
set_overhead(const char *str)
{
	return parse_ts(str, &overhead);
}

static int ATTRIBUTE_FORMAT((printf, 2, 3))
num_chars(FILE *dummy, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	int ret = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	return (unsigned int) MAX(ret, 0);
}

static int
print_percent_val(FILE *outf, const union count_summary_column_val *val,
		  const int w, const struct csct_config *fmt, bool last)
{
	switch (fmt->format) {
	case CSCT_PERCENT_PERCENT:
		return (!w ? num_chars : fprintf)
			(outf, !w ? "%1$.*2$f" : "%1$*3$.*2$f",
			 val->percent * 100, fmt->precision, w);
	case CSCT_PERCENT_PART:
		return (!w ? num_chars : fprintf)
			(outf, !w ? "%1$.*2$f" : "%1$*3$.*2$f",
			 val->percent, fmt->precision, w);
	default:
		return 0;
	}
}

static int
print_time_val(FILE *outf, const union count_summary_column_val *val,
	       const int w, const struct csct_config *fmt, bool last)
{
	switch (fmt->format) {
	case CSCT_TIME_S:
		return (!w ? num_chars : fprintf)
			(outf, !w ? "%1$" PRIu64 : "%1$*2$" PRId64,
			 (int64_t) val->t.tv_sec, w);
	case CSCT_TIME_MS:
		if (val->t.tv_sec) {
			return (!w ? num_chars : fprintf)
				(outf, !w ? "%1$" PRIu64 "%2$03u"
					  : "%1$*3$" PRId64 "%2$03u",
				 (int64_t) val->t.tv_sec,
				 (int32_t) val->t.tv_nsec / 1000000, w - 3);
		} else {
			return (!w ? num_chars : fprintf)
				(outf, !w ? "%1$u" : "%1$*2$u",
				 (int32_t) val->t.tv_nsec / 1000000, w);
		}
	case CSCT_TIME_US:
		if (val->t.tv_sec) {
			return (!w ? num_chars : fprintf)
				(outf, !w ? "%1$" PRIu64 "%2$06u"
					  : "%1$*3$" PRId64 "%2$06u",
				 (int64_t) val->t.tv_sec,
				 (int32_t) val->t.tv_nsec / 1000, w - 6);
		} else {
			return (!w ? num_chars : fprintf)
				(outf, !w ? "%1$u" : "%1$*2$u",
				 (int32_t) val->t.tv_nsec / 1000, w);
		}
	case CSCT_TIME_NS:
		if (val->t.tv_sec) {
			return (!w ? num_chars : fprintf)
				(outf, !w ? "%1$" PRIu64 "%2$09u"
					  : "%1$*3$" PRId64 "%2$09u",
				 (int64_t) val->t.tv_sec,
				 (int32_t) val->t.tv_nsec, w - 9);
		} else {
			return (!w ? num_chars : fprintf)
				(outf, !w ? "%1$u" : "%1$*2$u",
				 (int32_t) val->t.tv_nsec, w);
		}
	case CSCT_TIME_S_MS:
		return (!w ? num_chars : fprintf)
			(outf, !w ? "%1$" PRIu64 ".%2$03u"
				  : "%1$*3$" PRId64 ".%2$03u",
			 (int64_t) val->t.tv_sec,
			 (int32_t) val->t.tv_nsec / 1000000, w - 4);
	case CSCT_TIME_S_US:
		return (!w ? num_chars : fprintf)
			(outf, !w ? "%1$" PRIu64 ".%2$06u"
				  : "%1$*3$" PRId64 ".%2$06u",
			 (int64_t) val->t.tv_sec,
			 (int32_t) val->t.tv_nsec / 1000, w - 7);
	case CSCT_TIME_S_NS:
		return (!w ? num_chars : fprintf)
			(outf, !w ? "%1$" PRIu64 ".%2$09u"
				  : "%1$*3$" PRId64 ".%2$09u",
			 (int64_t) val->t.tv_sec, (int32_t) val->t.tv_nsec,
			 w - 10);
	default:
		return 0;
	}
}

static int
print_number_val(FILE *outf, const union count_summary_column_val *val,
		 const int w, const struct csct_config *fmt, bool last)
{
	switch (fmt->format) {
	case CSCT_NUMBER_NUMBER:
		if (!val->num && !fmt->print_zeroes) {
			if (!w)
				return 0;
			return fprintf(outf, "%*s", w, "");
		}
		return (!w ? num_chars : fprintf)
			(outf, !w ? "%1$" PRIu64 : "%1$*2$" PRId64,
			 val->num, w);
	default:
		return 0;
	};
}

static int
print_syscall_val(FILE *outf, const union count_summary_column_val *val,
		  const int w, const struct csct_config *fmt, bool last)
{
	switch (fmt->format) {
	case CSCT_SC_NAME_NAME:
		return (!w ? num_chars : fprintf)
			(outf, !w || last ? "%1$s" : "%1$-*2$s",
			 val->se.sc_name, w);
	case CSCT_SC_NAME_NUMBER:
		return (!w ? num_chars : fprintf)
			(outf, !w || last ? "%1$zu" : "%1$-*2$zu",
			 val->se.sc_num, w);
	case CSCT_SC_NAME_NAME_NUMBER: {
		size_t sc_len = strlen(val->se.sc_name);
		int num_len = num_chars(outf, "%zu", val->se.sc_num);

		if (!w)
			return sc_len + num_len + sizeof(" ()") - 1;

		return fprintf(outf, "%s (%zu)%*s",
			       val->se.sc_name, val->se.sc_num,
			       last ? 0 : w, "");
	}
	default:
		return 0;
	}
}

typedef int (*val_printer_t)(FILE *, const union count_summary_column_val *,
			     const int, const struct csct_config *, bool);

static inline int
print_val(FILE *outf, const union count_summary_column_val *val, const int w,
	  const struct csct_config *fmt, bool last)
{
	static const val_printer_t printers[] = {
		[CSCT_PERCENT] = print_percent_val,
		[CSCT_TIME] = print_time_val,
		[CSCT_NUMBER] = print_number_val,
		[CSCT_SC_NAME] = print_syscall_val,
	};

	enum count_summary_column_type type = COLUMN_TYPE(fmt->format);

	if (type > ARRAY_SIZE(printers) || !printers[type])
		return 0;

	return printers[type](outf, val, w, fmt, last);
}

static void
call_summary_pers(FILE *outf)
{
	static const double one = 1.0;

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
	union count_summary_column_val sc_max = { .se = { NULL, 0 } };

	/* sort, calculate statistics */
	indices = xcalloc(sizeof(indices[0]), nsyscalls);
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

		size_t sys_name_len = strlen(sysent[i].sys_name);
		if (sys_name_len > sc_name_max) {
			sc_name_max = sys_name_len;
			sc_max.se.sc_name = sysent[i].sys_name;
		}
		sc_max.se.sc_num = i;
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
		uint32_t flags;
	} cdesc[] = {
		[CSC_TIME_100S]  = { ARRSZ_PAIR("% time") - 1 },
		[CSC_TIME_MIN]   = { ARRSZ_PAIR("shortest") - 1 },
		[CSC_TIME_MAX]   = { ARRSZ_PAIR("longest") - 1 },
		/* Historical field sizes are preserved */
		[CSC_TIME_TOTAL] = { "seconds",    11 },
		[CSC_TIME_AVG]   = { "usecs/call", 11 },
		[CSC_CALLS]      = { "calls",       9 },
		[CSC_ERRORS]     = { "errors",      9 },
		[CSC_SC_NAME]    = { "syscall",    16, CF_L },
	};
	unsigned int cwidths[CSC_MAX];

	/* find the last column */
	for (size_t i = 0; i < ARRAY_SIZE(columns) && columns[i].column; ++i)
		last_column = i;

	/* calculate column widths */
#define W_(c_, v_) \
	case (c_): \
		cwidths[i] = MAX(cdesc[c_].sz, \
			(size_t) print_val(outf, \
				(const union count_summary_column_val *) &(v_),\
				0, columns + i, i == last_column)); \
		break
	for (size_t i = 0; i <= last_column; i++) {
		const size_t c = columns[i].column;

		switch (c) {
		W_(CSC_TIME_100S,  one);
		W_(CSC_TIME_TOTAL, tv_cum);
		W_(CSC_TIME_MIN,   *tv_min_max);
		W_(CSC_TIME_MAX,   *tv_max);
		W_(CSC_TIME_AVG,   *tv_avg_max);
		W_(CSC_CALLS,      call_cum);
		W_(CSC_ERRORS,     error_cum);
		W_(CSC_SC_NAME,    sc_max);
		}
	};
#undef W_

	/* header */
	for (size_t i = 0; i <= last_column; ++i) {
		const char *fmt = cdesc[columns[i].column].flags & CF_L
				  ? (i == last_column ? "%1$s" : "%1$-*2$s")
				  : "%1$*2$s";
		if (i)
			fputc(' ', outf);
		fprintf(outf, fmt, cdesc[columns[i].column].s, cwidths[i]);
	}
	fputc('\n', outf);

	/* divider */
	for (size_t i = 0; i <= last_column; ++i) {
		if (i)
			fputc(' ', outf);

		for (size_t j = 0; j < cwidths[i]; ++j)
			fputc('-', outf);
	}
	fputc('\n', outf);

	/* data output */
#define PC_(c_, v_) \
	case (c_): \
		print_val(outf, (union count_summary_column_val *) &(v_), \
			  cwidths[i], columns + i, i == last_column); \
		break
	for (size_t j = 0; j < nsyscalls; j++) {
		unsigned int idx = indices[j];
		struct call_counts *cc = &counts[idx];

		if (cc->calls == 0)
			continue;

		union count_summary_column_val sc = {
			.se = { sysent[idx].sys_name, idx } };

		percent = ts_float(&cc->time);
		/* else: float_tv_cum can be 0.0 too and we get 0/0 = NAN */
		if (percent != 0.0)
			   percent /= float_tv_cum;

		for (size_t i = 0; i <= last_column; ++i) {
			const size_t c = columns[i].column;
			if (i)
				fputc(' ', outf);

			switch (c) {
			PC_(CSC_TIME_100S,  percent);
			PC_(CSC_TIME_TOTAL, cc->time);
			PC_(CSC_TIME_MIN,   cc->time_min);
			PC_(CSC_TIME_MAX,   cc->time_max);
			PC_(CSC_TIME_AVG,   cc->time_avg);
			PC_(CSC_CALLS,      cc->calls);
			PC_(CSC_ERRORS,     cc->errors);
			PC_(CSC_SC_NAME,    sc);
			}
		}

		fputc('\n', outf);
	}

	free(indices);

	/* footer */
	for (size_t i = 0; i <= last_column; ++i) {
		if (i)
			fputc(' ', outf);

		for (size_t j = 0; j < cwidths[i]; ++j)
			fputc('-', outf);
	}
	fputc('\n', outf);

	struct timespec cum_tv_avg;
	ts_div(&cum_tv_avg, &tv_cum, call_cum);

	/* totals */
	for (size_t i = 0; i <= last_column; ++i) {
		const size_t c = columns[i].column;
		if (i)
			fputc(' ', outf);

		switch (c) {
		PC_(CSC_TIME_100S,  one);
		PC_(CSC_TIME_TOTAL, tv_cum);
		PC_(CSC_TIME_MIN,   tv_min);
		PC_(CSC_TIME_MAX,   tv_max);
		PC_(CSC_TIME_AVG,   cum_tv_avg);
		PC_(CSC_CALLS,      call_cum);
		PC_(CSC_ERRORS,     error_cum);
		case CSC_SC_NAME:
			fprintf(outf, i == last_column ? "%1$s" : "%1$-*2$s",
			       "total", cwidths[i]);
		}
	}
	fputc('\n', outf);

#undef PC_
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
