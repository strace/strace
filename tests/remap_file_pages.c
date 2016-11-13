/*
 * Check decoding of remap_file_pages syscall.
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
#include <asm/unistd.h>

#ifdef __NR_remap_file_pages

# include <stdio.h>
# include <sys/mman.h>
# include <unistd.h>

int
main(void)
{
	const unsigned long addr = (unsigned long) 0xfacefeeddeadbeefULL;
	const unsigned long size = (unsigned long) 0xdefaced1bad2f00dULL;
	const unsigned long prot = PROT_READ|PROT_WRITE|PROT_EXEC;
	const unsigned long pgoff = (unsigned long) 0xcaf3babebad4deedULL;
	const unsigned long flags = MAP_PRIVATE|MAP_ANONYMOUS;

	long rc = syscall(__NR_remap_file_pages, addr, size, prot, pgoff, flags);
	printf("remap_file_pages(%#lx, %lu, %s, %lu, %s) = %ld %s (%m)\n",
	       addr, size, "PROT_READ|PROT_WRITE|PROT_EXEC", pgoff,
	       "MAP_PRIVATE|MAP_ANONYMOUS", rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_remap_file_pages")

#endif
