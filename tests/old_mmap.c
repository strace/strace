/*
 * Check decoding of "old mmap" edition of mmap syscall.
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

/*
 * On s390x and m68k, this is the mmap syscall used by glibc, so,
 * from one side, it's already covered by another test, and, from another side,
 * it would require additional efforts to filter out mmap calls made by glibc.
 */

#if defined __NR_mmap && \
(   defined __arm__ \
 || defined __i386__ \
 || (defined __s390__ && !defined __s390x__) \
)

# include <stdio.h>
# include <string.h>
# include <sys/mman.h>
# include <unistd.h>

int
main(void)
{
	long rc = syscall(__NR_mmap, 0);
	printf("mmap(NULL) = %ld %s (%m)\n", rc, errno2name());

	const unsigned int args1_c[6] = {
		0xdeadbeef,		/* addr */
		0xfacefeed,		/* len */
		PROT_READ|PROT_EXEC,	/* prot */
		MAP_FILE|MAP_FIXED,	/* flags */
		-2U,			/* fd */
		0xbadc0ded		/* offset */
	};
	const unsigned int page_size = get_page_size();
	const unsigned int args2_c[6] = {
		0,
		page_size,
		PROT_READ|PROT_WRITE,
		MAP_PRIVATE|MAP_ANONYMOUS,
		-1U,
		0xfaced000 & -page_size
	};
	void *args = tail_memdup(args1_c, sizeof(args1_c));

	rc = syscall(__NR_mmap, args);
	printf("mmap(%#x, %u, PROT_READ|PROT_EXEC, MAP_FILE|MAP_FIXED"
	       ", %d, %#x) = %ld %s (%m)\n",
	       args1_c[0], args1_c[1], args1_c[4], args1_c[5],
	       rc, errno2name());

	memcpy(args, args2_c, sizeof(args2_c));
	rc = syscall(__NR_mmap, args);
	printf("mmap(NULL, %u, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS"
	       ", %d, %#x) = %#lx\n",
	       args2_c[1], args2_c[4], args2_c[5], rc);

	void *addr = (void *) rc;
	if (mprotect(addr, page_size, PROT_NONE))
		perror_msg_and_fail("mprotect(%p, %u, PROT_NONE)",
				    addr, page_size);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mmap && (__arm__ || __i386__"
		    " || (__s390__ && !__s390x__))")

#endif
