/*
 * Print time_t and nanoseconds in symbolic format.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <time.h>

static void
print_time_t_ex(const time_t t, const unsigned long long part_sec,
		const unsigned int max_part_sec, const int width,
		const int comment)
{

	if ((!t && !part_sec) || part_sec > max_part_sec)
		return;

	const struct tm *const p = localtime(&t);
	char buf[256];
	if (!p || !strftime(buf, sizeof(buf), "%FT%T", p))
		return;

	if (comment)
		fputs(" /* ", stdout);

	fputs(buf, stdout);

	if (part_sec)
		printf(".%0*llu", width, part_sec);

	if (strftime(buf, sizeof(buf), "%z", p))
		fputs(buf, stdout);

	if (comment)
		fputs(" */", stdout);
}

void
print_time_t_nsec(const time_t t, const unsigned long long nsec, int comment)
{
	print_time_t_ex(t, nsec, 999999999, 9, comment);
}

void
print_time_t_usec(const time_t t, const unsigned long long usec, int comment)
{
	print_time_t_ex(t, usec, 999999, 6, comment);
}
