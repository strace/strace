/*
 * Print time_t and nanoseconds in symbolic format.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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

	return;
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
