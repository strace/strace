/*
 * This file is part of utime strace test.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <time.h>
#include <utime.h>
#include <errno.h>
#include <stdio.h>

static void
print_tm(const struct tm * const p)
{
	printf("%02d/%02d/%02d-%02d:%02d:%02d",
	       p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
	       p->tm_hour, p->tm_min, p->tm_sec);
}

int
main(void)
{
	int rc = utime("", NULL);
	printf("utime(\"\", NULL) = %s\n", sprintrc(rc));

	const time_t t = time(NULL);
	const struct tm * const p = localtime(&t);
	const struct utimbuf u = { .actime = t, .modtime = t };
	const struct utimbuf const *tail_u = tail_memdup(&u, sizeof(u));

	rc = utime("utime\nfilename", tail_u);
	const char *errstr = sprintrc(rc);
	printf("utime(\"utime\\nfilename\", [");
	print_tm(p);
	printf(", ");
	print_tm(p);
	printf("]) = %s\n", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
