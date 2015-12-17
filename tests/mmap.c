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

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <sys/mman.h>

int
main(void)
{
	const intmax_t pagesize = sysconf(_SC_PAGESIZE);
	const unsigned long length = pagesize * 3;
	const int fd = -1;
	off_t offset;
	void *addr, *p;

#if ULONG_MAX > 4294967295UL
	offset = 0xcafedeadbeef000 & -pagesize;
	addr = (void *) (uintmax_t) (0xfacefeed000 & -pagesize);
#else
	offset = 0xdeadbeef000 & -pagesize;
	addr = (void *) (unsigned int) (0xfaced000 & -pagesize);
#endif

	p = mmap(addr, length, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS, fd, offset);
	if (p == MAP_FAILED ||
	    mprotect(p, length, PROT_NONE) ||
	    munmap(p, length))
		return 77;

	if (sizeof(offset) == sizeof(int))
		printf("mmap2?\\(%p, %lu, PROT_READ\\|PROT_WRITE, "
		       "MAP_PRIVATE\\|MAP_ANONYMOUS, %d, %#x\\) = %p\n",
		       addr, length, fd, (unsigned int) offset, p);
	else
		printf("(mmap2?|old_mmap)\\(%p, %lu, PROT_READ\\|PROT_WRITE, "
		       "MAP_PRIVATE\\|MAP_ANONYMOUS, %d, %#jx\\) = %p\n",
		       addr, length, fd, (uintmax_t) offset, p);
	printf("mprotect\\(%p, %lu, PROT_NONE\\) += 0\n", p, length);
	printf("munmap\\(%p, %lu\\) += 0\n", p, length);
	return 0;
}
