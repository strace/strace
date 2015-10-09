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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "flock.h"

#define FILE_LEN 4096

static void
test_flock(int nr, const char *name)
{
	struct_kernel_flock fl = {
		.l_type = F_RDLCK,
		.l_start = (off_t) 0xdefaced1facefeed,
		.l_len = (off_t) 0xdefaced2cafef00d
	};
	int rc;

	syscall(nr, -1, F_SETLK, &fl);
	printf("%s(-1, F_SETLK, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=%jd, l_len=%jd}) = -1 %s\n",
	       name, (intmax_t) fl.l_start, (intmax_t) fl.l_len,
	       errno == EBADF ? "EBADF (Bad file descriptor)" :
				"EINVAL (Invalid argument)");

	fl.l_start = 0;
	fl.l_len = FILE_LEN;

	rc = syscall(nr, 0, F_SETLK, &fl);
	printf("%s(0, F_SETLK, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = %s\n",
	       name, FILE_LEN, rc ? "-1 EINVAL (Invalid argument)" : "0");

	if (rc)
		return;

	syscall(nr, 0, F_GETLK, &fl);
	printf("%s(0, F_GETLK, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d, l_pid=0}) = 0\n", name, FILE_LEN);

	syscall(nr, 0, F_SETLK, &fl);
	printf("%s(0, F_SETLK, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = 0\n", name, FILE_LEN);

}

static void
test_flock64(int nr, const char *name)
{
	struct_kernel_flock64 fl = {
		.l_type = F_RDLCK,
		.l_start = 0xdefaced1facefeed,
		.l_len = 0xdefaced2cafef00d
	};
	int rc;

	syscall(nr, -1, F_SETLK64, &fl);
	printf("%s(-1, F_SETLK64, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=%jd, l_len=%jd}) = -1 %s\n",
	       name, (intmax_t) fl.l_start, (intmax_t) fl.l_len,
	       errno == EBADF ? "EBADF (Bad file descriptor)" :
				"EINVAL (Invalid argument)");

	fl.l_start = 0;
	fl.l_len = FILE_LEN;

	rc = syscall(nr, 0, F_SETLK64, &fl);
	printf("%s(0, F_SETLK64, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = %s\n",
	       name, FILE_LEN, rc ? "-1 EINVAL (Invalid argument)" : "0");

	if (rc)
		return;

	syscall(nr, 0, F_GETLK64, &fl);
	printf("%s(0, F_GETLK64, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d, l_pid=0}) = 0\n", name, FILE_LEN);

	syscall(nr, 0, F_SETLK64, &fl);
	printf("%s(0, F_SETLK64, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = 0\n", name, FILE_LEN);
}

int
main(void)
{
	char fname[] = "struct_flock_XXXXXX";

	(void) close(0);
	assert(mkstemp(fname) == 0);
	assert(unlink(fname) == 0);
	assert(ftruncate(0, FILE_LEN) == 0);

#ifdef __NR_fcntl
	test_flock(__NR_fcntl, "fcntl");
	test_flock64(__NR_fcntl, "fcntl");
#endif
#ifdef __NR_fcntl64
	test_flock(__NR_fcntl64, "fcntl64");
	test_flock64(__NR_fcntl64, "fcntl64");
#endif

	puts("+++ exited with 0 +++");
	return 0;
}
