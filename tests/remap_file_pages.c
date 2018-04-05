/*
 * Check decoding of remap_file_pages syscall.
 *
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
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

#ifdef __NR_remap_file_pages

# include <stdio.h>
# include <stdint.h>
# include <unistd.h>
# include <linux/mman.h>

static const char *errstr;

static long
k_remap_file_pages(const kernel_ulong_t addr,
		   const kernel_ulong_t size,
		   const kernel_ulong_t prot,
		   const kernel_ulong_t pgoff,
		   const kernel_ulong_t flags)
{
	const long rc = syscall(__NR_remap_file_pages,
				addr, size, prot, pgoff, flags);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	kernel_ulong_t addr = (kernel_ulong_t) 0xfacefeeddeadbeefULL;
	kernel_ulong_t size = (kernel_ulong_t) 0xdefaced1bad2f00dULL;
	kernel_ulong_t prot = PROT_READ|PROT_WRITE|PROT_EXEC;
	kernel_ulong_t pgoff = (kernel_ulong_t) 0xcaf3babebad4deedULL;
	kernel_ulong_t flags = MAP_PRIVATE|MAP_ANONYMOUS;

	k_remap_file_pages(addr, size, prot, pgoff, flags);
	printf("remap_file_pages(%#jx, %ju, %s, %ju, %s) = %s\n",
	       (uintmax_t) addr, (uintmax_t) size,
	       "PROT_READ|PROT_WRITE|PROT_EXEC",
	       (uintmax_t) pgoff, "MAP_PRIVATE|MAP_ANONYMOUS", errstr);

#ifdef MAP_HUGETLB
# ifndef MAP_HUGE_2MB
#  ifndef MAP_HUGE_SHIFT
#   define MAP_HUGE_SHIFT 26
#  endif
#  define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
# endif /* !MAP_HUGE_2MB */
	addr = (kernel_ulong_t) 0xfacefeeddeadf00dULL;
	size = (kernel_ulong_t) 0xdefaced1bad2beefULL;
	prot = (kernel_ulong_t) 0xdefaced00000000ULL | PROT_NONE;
	flags = MAP_TYPE | MAP_FIXED | MAP_NORESERVE | MAP_HUGETLB | MAP_HUGE_2MB;

	k_remap_file_pages(addr, size, prot, pgoff, flags);
	printf("remap_file_pages(%#jx, %ju, %s, %ju"
/*
 * HP PA-RISC is the only architecture that has MAP_TYPE defined to 0x3, which
 * is also used for MAP_SHARED_VALIDATE since Linux commit v4.15-rc1~71^2^2~23.
 */
# ifdef __hppa__
	       ", MAP_SHARED_VALIDATE"
# else
	       ", 0xf /* MAP_??? */"
# endif
	       "|MAP_FIXED|MAP_NORESERVE|MAP_HUGETLB|21<<MAP_HUGE_SHIFT)"
	       " = %s\n",
	       (uintmax_t) addr, (uintmax_t) size,
	       prot == PROT_NONE ? "PROT_NONE" :
				   "0xdefaced00000000 /* PROT_??? */",
	       (uintmax_t) pgoff, errstr);
#endif /* MAP_HUGETLB */

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_remap_file_pages")

#endif
