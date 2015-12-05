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

#include <sys/syscall.h>

#ifdef __NR_fcntl64

# define TEST_SYSCALL_NAME fcntl64
# include "struct_flock.c"

#define TEST_FLOCK64_EINVAL(cmd) test_flock64_einval(cmd, #cmd)

static void
test_flock64_einval(const int cmd, const char *name)
{
	struct_kernel_flock64 fl = {
		.l_type = F_RDLCK,
		.l_start = 0xdefaced1facefeed,
		.l_len = 0xdefaced2cafef00d
	};
	syscall(TEST_SYSCALL_NR, 0, cmd, &fl);
	printf("%s(0, %s, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=%jd, l_len=%jd}) = %s\n", TEST_SYSCALL_STR, name,
	       (intmax_t) fl.l_start, (intmax_t) fl.l_len, EINVAL_STR);
}

static void
test_flock64(void)
{
	TEST_FLOCK64_EINVAL(F_SETLK64);
	TEST_FLOCK64_EINVAL(F_SETLKW64);
#ifdef F_OFD_SETLK
	TEST_FLOCK64_EINVAL(F_OFD_SETLK);
	TEST_FLOCK64_EINVAL(F_OFD_SETLKW);
#endif

	struct_kernel_flock64 fl = {
		.l_type = F_RDLCK,
		.l_len = FILE_LEN
	};
	int rc = syscall(TEST_SYSCALL_NR, 0, F_SETLK64, &fl);
	printf("%s(0, F_SETLK64, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = %s\n",
	       TEST_SYSCALL_STR, FILE_LEN, rc ? EINVAL_STR : "0");

	if (rc)
		return;

	syscall(TEST_SYSCALL_NR, 0, F_GETLK64, &fl);
	printf("%s(0, F_GETLK64, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d, l_pid=0}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);

	syscall(TEST_SYSCALL_NR, 0, F_SETLK64, &fl);
	printf("%s(0, F_SETLK64, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);
}

int
main(void)
{
	if (create_sample())
		return 77;

	test_flock();
	test_flock64();

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
