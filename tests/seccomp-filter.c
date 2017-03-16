/*
 * Check decoding of seccomp SECCOMP_SET_MODE_FILTER.
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

#include "tests.h"

#include <stdio.h>
#include <asm/unistd.h>
#include <unistd.h>

#ifdef HAVE_LINUX_SECCOMP_H
# include <linux/seccomp.h>
#endif
#ifdef HAVE_LINUX_FILTER_H
# include <linux/filter.h>
#endif

#if defined __NR_seccomp && defined SECCOMP_SET_MODE_FILTER

# define N 7

int
main(void)
{
	struct sock_filter *const filter = tail_alloc(sizeof(*filter) * N);
	const void *const efault = tail_alloc(1);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct sock_fprog, prog);
	long rc;

	prog->filter = filter;
	prog->len = N;
	rc = syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, -1, prog);
	printf("seccomp(SECCOMP_SET_MODE_FILTER, %s, {len=%u, filter=%p})"
	       " = %ld %s (%m)\n", "SECCOMP_FILTER_FLAG_TSYNC|0xfffffffe",
	       prog->len, prog->filter, rc, errno2name());

	rc = syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, -2L, efault);
	printf("seccomp(SECCOMP_SET_MODE_FILTER, %s, %p) = %ld %s (%m)\n",
	       "0xfffffffe /* SECCOMP_FILTER_FLAG_??? */",
	       efault, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_seccomp && SECCOMP_SET_MODE_FILTER")

#endif
