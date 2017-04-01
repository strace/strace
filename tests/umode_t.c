/*
 * Check decoding of umode_t type syscall arguments.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <unistd.h>
#include <sys/stat.h>

#ifndef TEST_SYSCALL_PREFIX_ARGS
# define TEST_SYSCALL_PREFIX_ARGS
#endif
#ifndef TEST_SYSCALL_PREFIX_STR
# define TEST_SYSCALL_PREFIX_STR ""
#endif

static const char *sample;

static void
test_syscall(unsigned short mode)
{
	unsigned long lmode = (unsigned long) 0xffffffffffff0000ULL | mode;
	long rc = syscall(TEST_SYSCALL_NR, TEST_SYSCALL_PREFIX_ARGS
			  sample, lmode);

	if (mode <= 07)
		printf("%s(%s\"%s\", 00%d) = %ld %s (%m)\n",
		       TEST_SYSCALL_STR, TEST_SYSCALL_PREFIX_STR,
		       sample, (int) mode, rc, errno2name());
	else
		printf("%s(%s\"%s\", %#03ho) = %ld %s (%m)\n",
		       TEST_SYSCALL_STR, TEST_SYSCALL_PREFIX_STR,
		       sample, mode, rc, errno2name());
}

int
main(int ac, char **av)
{
	sample = av[0];
	test_syscall(0);
	test_syscall(0xffff);
	test_syscall(06);
	test_syscall(060);
	test_syscall(0600);
	test_syscall(024);
	test_syscall(S_IFREG);
	test_syscall(S_IFDIR | 06);
	test_syscall(S_IFLNK | 060);
	test_syscall(S_IFIFO | 0600);
	test_syscall(S_IFCHR | 024);
	test_syscall((0xffff & ~S_IFMT) | S_IFBLK);

	puts("+++ exited with 0 +++");
	return 0;
}
