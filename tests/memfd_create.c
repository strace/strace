/*
 * Check decoding of memfd_create syscall.
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
#include <asm/unistd.h>
#include "scno.h"

#ifdef __NR_memfd_create

# include <stdio.h>
# include <stdint.h>
# include <unistd.h>

# ifdef HAVE_LINUX_MEMFD_H
#  include <linux/memfd.h>
# endif

# ifndef MFD_HUGE_SHIFT
#  define MFD_HUGE_SHIFT 26
# endif

# ifndef MFD_HUGE_MASK
#  define MFD_HUGE_MASK 0x3f
# endif

static const char *errstr;

static long
k_memfd_create(const kernel_ulong_t name, const kernel_ulong_t flags)
{
	const long rc = syscall(__NR_memfd_create, name, flags);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	const size_t size = 255 - (sizeof("memfd:") - 1) + 1;
	char *pattern = tail_alloc(size);
	fill_memory_ex(pattern, size, '0', 10);

	k_memfd_create((uintptr_t) pattern, 0);
	printf("memfd_create(\"%.*s\"..., 0) = %s\n",
	       (int) size - 1, pattern, errstr);

	kernel_ulong_t flags = (kernel_ulong_t) 0xfacefeed00000007ULL;
	k_memfd_create((uintptr_t) pattern, flags);
	printf("memfd_create(\"%.*s\"..., %s) = %s\n",
	       (int) size - 1, pattern,
	       "MFD_CLOEXEC|MFD_ALLOW_SEALING|MFD_HUGETLB",
	       errstr);

	pattern[size - 1] = '\0';
	flags = 30 << MFD_HUGE_SHIFT;
	k_memfd_create((uintptr_t) pattern, flags);
	printf("memfd_create(\"%s\", 30<<MFD_HUGE_SHIFT) = %s\n",
	       pattern, errstr);

	pattern += size - 1;
	flags = (kernel_ulong_t) -1ULL;
	k_memfd_create(0, flags);
	flags = -1U & ~(7 | (MFD_HUGE_MASK << MFD_HUGE_SHIFT));
	printf("memfd_create(NULL, MFD_CLOEXEC|MFD_ALLOW_SEALING|MFD_HUGETLB"
	       "|%#x|%u<<MFD_HUGE_SHIFT) = %s\n",
	       (unsigned int) flags, MFD_HUGE_MASK, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_memfd_create")

#endif
