/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>

#if defined HAVE_UTIMENSAT \
 && defined AT_FDCWD && defined AT_SYMLINK_NOFOLLOW \
 && defined UTIME_NOW && defined UTIME_OMIT

static void
print_ts(const struct timespec *ts)
{
	printf("{%ju, %ju}", (uintmax_t) ts->tv_sec, (uintmax_t) ts->tv_nsec);
}

int
main(void)
{
	struct timeval tv;
	struct timespec ts[2];

	if (gettimeofday(&tv, NULL))
		return 77;

	ts[0].tv_sec = tv.tv_sec;
	ts[0].tv_nsec = tv.tv_usec;
	ts[1].tv_sec = tv.tv_sec - 1;
	ts[1].tv_nsec = tv.tv_usec + 1;
	if (!utimensat(AT_FDCWD, "utimensat\nfilename", ts,
	     AT_SYMLINK_NOFOLLOW))
		return 77;

	#define PREFIX "utimensat(AT_FDCWD, \"utimensat\\nfilename\", ["

	printf(PREFIX);
	print_ts(&ts[0]);
	printf(", ");
	print_ts(&ts[1]);
	puts("], AT_SYMLINK_NOFOLLOW) = -1 ENOENT (No such file or directory)");

	ts[0].tv_nsec = UTIME_NOW;
	ts[1].tv_nsec = UTIME_OMIT;
	if (!utimensat(AT_FDCWD, "utimensat\nfilename", ts,
	     AT_SYMLINK_NOFOLLOW))
		return 77;

	printf(PREFIX);
	puts("UTIME_NOW, UTIME_OMIT], AT_SYMLINK_NOFOLLOW) = -1 ENOENT (No such file or directory)");
	puts("+++ exited with 0 +++");

	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
