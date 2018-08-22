/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
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
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <sys/mman.h>

int
main(int ac, char **av)
{
	const char *const name = ac > 1 ? av[1] : "mmap";
	const intmax_t pagesize = get_page_size();
	const unsigned long length1 = pagesize * 6;
	const unsigned long length2 = pagesize * 3;
	const unsigned long length3 = pagesize * 2;
	const int fd = -1;
	off_t offset;
	void *addr, *p;

#if ULONG_MAX > 4294967295UL
	offset = 0xcafedeadbeef000ULL & -pagesize;
	addr = (void *) (uintmax_t) (0xfacefeed000 & -pagesize);
#else
	offset = 0xdeadbeef000ULL & -pagesize;
	addr = (void *) (unsigned int) (0xfaced000 & -pagesize);
#endif
	const uintmax_t uoffset =
	       sizeof(offset) == sizeof(int) ? (uintmax_t) (unsigned int) offset
					     : (uintmax_t) offset;

	(void) close(0);
	(void) close(0);
#if XLAT_RAW
	printf("%s(NULL, 0, %#x, %#x, 0, 0) = -1 EBADF (%m)\n",
	       name, PROT_NONE, MAP_FILE);
#elif XLAT_VERBOSE
	printf("%s(NULL, 0, %#x /* PROT_NONE */, %#x /* MAP_FILE */, 0, 0) "
	       "= -1 EBADF (%m)\n",
	       name, PROT_NONE, MAP_FILE);
#else
	printf("%s(NULL, 0, PROT_NONE, MAP_FILE, 0, 0) = -1 EBADF (%m)\n",
	       name);
#endif
	mmap(NULL, 0, PROT_NONE, MAP_FILE, 0, 0);

	p = mmap(addr, length1, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS, fd, offset);
	if (MAP_FAILED == p)
		perror_msg_and_fail("mmap");
#if XLAT_RAW
	printf("%s(%p, %lu, %#x, "
	       "%#x|%#x, %d, %#jx) = %p\n",
	       name, addr, length1, PROT_READ | PROT_WRITE, MAP_PRIVATE,
	       MAP_ANONYMOUS, fd, uoffset, p);
#elif XLAT_VERBOSE
	printf("%s(%p, %lu, %#x /* PROT_READ|PROT_WRITE */, "
	       "%#x /* MAP_PRIVATE */|%#x /* MAP_ANONYMOUS */, %d, %#jx) "
	       "= %p\n",
	       name, addr, length1, PROT_READ | PROT_WRITE, MAP_PRIVATE,
	       MAP_ANONYMOUS, fd, uoffset, p);
#else
	printf("%s(%p, %lu, PROT_READ|PROT_WRITE, "
	       "MAP_PRIVATE|MAP_ANONYMOUS, %d, %#jx) = %p\n",
	       name, addr, length1, fd, uoffset, p);
#endif

	if (msync(p, length1, MS_SYNC))
		perror_msg_and_fail("msync");
#if XLAT_RAW
	printf("msync(%p, %lu, %#x) = 0\n", p, length1, MS_SYNC);
#elif XLAT_VERBOSE
	printf("msync(%p, %lu, %#x /* MS_SYNC */) = 0\n", p, length1, MS_SYNC);
#else
	printf("msync(%p, %lu, MS_SYNC) = 0\n", p, length1);
#endif

	if (mprotect(p, length1, PROT_NONE))
		perror_msg_and_fail("mprotect");
#if XLAT_RAW
	printf("mprotect(%p, %lu, %#x) = 0\n", p, length1, PROT_NONE);
#elif XLAT_VERBOSE
	printf("mprotect(%p, %lu, %#x /* PROT_NONE */) = 0\n",
	       p, length1, PROT_NONE);
#else
	printf("mprotect(%p, %lu, PROT_NONE) = 0\n", p, length1);
#endif

	addr = mremap(p, length1, length2, 0);
	if (MAP_FAILED == addr)
		perror_msg_and_fail("mremap");
	printf("mremap(%p, %lu, %lu, 0) = %p\n", p, length1, length2, addr);

	p =  mremap(addr, length2, length3, MREMAP_MAYMOVE | MREMAP_FIXED,
		    addr + length2);
	if (MAP_FAILED == p)
		perror_msg_and_fail("mremap");
#if XLAT_RAW
	printf("mremap(%p, %lu, %lu, %#x, %p) = %p\n",
	       addr, length2, length3, MREMAP_MAYMOVE | MREMAP_FIXED,
	       addr + length2, p);
#elif XLAT_VERBOSE
	printf("mremap(%p, %lu, %lu, %#x /* MREMAP_MAYMOVE|MREMAP_FIXED */"
	       ", %p) = %p\n",
	       addr, length2, length3, MREMAP_MAYMOVE | MREMAP_FIXED,
	       addr + length2, p);
#else
	printf("mremap(%p, %lu, %lu, MREMAP_MAYMOVE|MREMAP_FIXED"
	       ", %p) = %p\n", addr, length2, length3, addr + length2, p);
#endif

	if (munmap(p, length3))
		perror_msg_and_fail("munmap");
	printf("munmap(%p, %lu) = 0\n", p, length3);

	printf("mlockall(");
#if XLAT_RAW
	printf("%#x", MCL_FUTURE);
#elif XLAT_VERBOSE
	printf("%#x /* MCL_FUTURE */", MCL_FUTURE);
#else
	printf("MCL_FUTURE");
#endif
	printf(") = %s\n", sprintrc(mlockall(MCL_FUTURE)));

	puts("+++ exited with 0 +++");
	return 0;
}
