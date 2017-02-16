/*
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "flock.h"

#define FILE_LEN 4096

#define TEST_FLOCK_EINVAL(cmd) test_flock_einval(cmd, #cmd)

#ifdef HAVE_TYPEOF
# define TYPEOF_FLOCK_OFF_T typeof(((struct_kernel_flock *) NULL)->l_len)
#else
# define TYPEOF_FLOCK_OFF_T off_t
#endif

static long
invoke_test_syscall(const unsigned int cmd, void *const p)
{
	const kernel_ulong_t fd = F8ILL_KULONG_MASK;
	const kernel_ulong_t op = F8ILL_KULONG_MASK | cmd;

	return syscall(TEST_SYSCALL_NR, fd, op, (unsigned long) p);
}

static void
test_flock_einval(const int cmd, const char *name)
{
	struct_kernel_flock fl = {
		.l_type = F_RDLCK,
		.l_start = (TYPEOF_FLOCK_OFF_T) 0xdefaced1facefeedULL,
		.l_len = (TYPEOF_FLOCK_OFF_T) 0xdefaced2cafef00dULL
	};
	long rc = invoke_test_syscall(cmd, &fl);
	printf("%s(0, %s, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=%jd, l_len=%jd}) = %s\n", TEST_SYSCALL_STR, name,
	       (intmax_t) fl.l_start, (intmax_t) fl.l_len, sprintrc(rc));
}

static void
test_flock(void)
{
	TEST_FLOCK_EINVAL(F_SETLK);
	TEST_FLOCK_EINVAL(F_SETLKW);

	struct_kernel_flock fl = {
		.l_type = F_RDLCK,
		.l_len = FILE_LEN
	};
	long rc = invoke_test_syscall(F_SETLK, &fl);
	printf("%s(0, F_SETLK, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = %s\n",
	       TEST_SYSCALL_STR, FILE_LEN, sprintrc(rc));
	if (rc)
		return;

	invoke_test_syscall(F_GETLK, &fl);
	printf("%s(0, F_GETLK, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d, l_pid=0}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);

	invoke_test_syscall(F_SETLK, &fl);
	printf("%s(0, F_SETLK, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);
}

static void
create_sample(void)
{
	char fname[] = TEST_SYSCALL_STR "_XXXXXX";

	(void) close(0);
	if (mkstemp(fname))
		perror_msg_and_fail("mkstemp: %s", fname);
	if (unlink(fname))
		perror_msg_and_fail("unlink: %s", fname);
	if (ftruncate(0, FILE_LEN))
		perror_msg_and_fail("ftruncate");
}
