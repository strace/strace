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

/* Per-syscall stats structure */
struct call_counts {
	/* time may be total latency or system time */
	struct timespec time;
	unsigned int calls, errors;
};

static struct call_counts *countv[SUPPORTED_PERSONALITIES];
#define counts (countv[current_personality])

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

	if (count_wallclock) {
		/* wall clock time spent while in syscall */
		struct timespec wts;
		ts_sub(&wts, syscall_exiting_ts, &tcp->etime);

		ts_add(&cc->time, &cc->time, &wts);
	} else {
		/* system CPU time spent while in syscall */
		ts_add(&cc->time, &cc->time, &tcp->dtime);
	}
}

static int
time_cmp(void *a, void *b)
{
	return -ts_cmp(&counts[*((int *) a)].time,
		       &counts[*((int *) b)].time);
}

static int
syscall_cmp(void *a, void *b)
{
	const char *a_name = sysent[*((int *) a)].sys_name;
	const char *b_name = sysent[*((int *) b)].sys_name;
	return strcmp(a_name ? a_name : "", b_name ? b_name : "");
}

static int
count_cmp(void *a, void *b)
{
	int     m = counts[*((int *) a)].calls;
	int     n = counts[*((int *) b)].calls;

	return (m < n) ? 1 : (m > n) ? -1 : 0;
}

static int (*sortfun)();

void
set_sortby(const char *sortby)
{
	if (strcmp(sortby, "time") == 0)
		sortfun = time_cmp;
	else if (strcmp(sortby, "calls") == 0)
		sortfun = count_cmp;
	else if (strcmp(sortby, "name") == 0)
		sortfun = syscall_cmp;
	else if (strcmp(sortby, "nothing") == 0)
		sortfun = NULL;
	else {
		error_msg_and_help("invalid sortby: '%s'", sortby);
	}
}

void set_overhead(int n)
{
	overhead.tv_sec = n / 1000000;
	overhead.tv_nsec = n % 1000000 * 1000;
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
		ts_mul(&dtv, &overhead, counts[i].calls);
		ts_sub(&counts[i].time, &counts[i].time, &dtv);
		if (counts[i].time.tv_sec < 0 || counts[i].time.tv_nsec < 0)
			counts[i].time.tv_sec = counts[i].time.tv_nsec = 0;
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
