/*
 * Check decoding of "old mmap" edition of mmap syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

/*
 * On s390x and m68k, this is the mmap syscall used by glibc, so,
 * from one side, it's already covered by another test, and, from another side,
 * it would require additional efforts to filter out mmap calls made by glibc.
 */

#if defined __NR_mmap \
	&& (defined __arm__ || defined __i386__ || defined __m68k__ \
		|| defined __s390__ || defined __s390x__) \
	&& (defined PATH_TRACING || !(defined __s390x__ || defined __m68k__))

# include <stdio.h>
# include <string.h>
# include <sys/mman.h>
# include <unistd.h>

# ifndef TEST_FD
#  define TEST_FD -2LU
# endif

int
main(void)
{
	long rc = syscall(__NR_mmap, 0);
# ifndef PATH_TRACING
	printf("mmap(NULL) = %ld %s (%m)\n", rc, errno2name());
# endif

	const unsigned long args1_c[6] = {
		(unsigned long) 0xbadc0deddeadbeefULL,	/* addr */
		(unsigned long) 0xdeefacedfacefeedULL,	/* len */
		PROT_READ|PROT_EXEC,	/* prot */
		MAP_FILE|MAP_FIXED,	/* flags */
		TEST_FD,		/* fd */
		(unsigned long) 0xdecaffedbadc0dedULL	/* offset */
	};
	const unsigned long page_size = get_page_size();
	const unsigned long args2_c[6] = {
		0,
		page_size,
		PROT_READ|PROT_WRITE,
		MAP_PRIVATE|MAP_ANONYMOUS,
		-1LU,
		(unsigned long) 0xda7a1057faced000ULL & -page_size
	};
	void *args = tail_memdup(args1_c, sizeof(args1_c));

	rc = syscall(__NR_mmap, args);
# if XLAT_RAW
	printf("mmap(%#lx, %lu, %#x, %#x|%#x, %d, %#lx) = %ld %s (%m)\n",
	       args1_c[0], args1_c[1], PROT_READ|PROT_EXEC, MAP_FILE, MAP_FIXED,
	       (int) args1_c[4], args1_c[5], rc, errno2name());
# elif XLAT_VERBOSE
	printf("mmap(%#lx, %lu, %#x /* PROT_READ|PROT_EXEC */"
	       ", %#x /* MAP_FILE */|%#x /* MAP_FIXED */"
	       ", %d, %#lx) = %ld %s (%m)\n",
	       args1_c[0], args1_c[1], PROT_READ|PROT_EXEC, MAP_FILE, MAP_FIXED,
	       (int) args1_c[4], args1_c[5], rc, errno2name());
# else
	printf("mmap(%#lx, %lu, PROT_READ|PROT_EXEC, MAP_FILE|MAP_FIXED"
	       ", %d, %#lx) = %ld %s (%m)\n",
	       args1_c[0], args1_c[1], (int) args1_c[4], args1_c[5],
	       rc, errno2name());
# endif

	memcpy(args, args2_c, sizeof(args2_c));
	rc = syscall(__NR_mmap, args);
# ifndef PATH_TRACING
#  if XLAT_RAW
	printf("mmap(NULL, %lu, %#x, %#x|%#x, %d, %#lx) = %#lx\n",
	       args2_c[1], PROT_READ|PROT_WRITE, MAP_PRIVATE, MAP_ANONYMOUS,
	       (int) args2_c[4], args2_c[5], rc);
#  elif XLAT_VERBOSE
	printf("mmap(NULL, %lu, %#x /* PROT_READ|PROT_WRITE */"
	       ", %#x /* MAP_PRIVATE */|%#x /* MAP_ANONYMOUS */"
	       ", %d, %#lx) = %#lx\n",
	       args2_c[1], PROT_READ|PROT_WRITE, MAP_PRIVATE, MAP_ANONYMOUS,
	       (int) args2_c[4], args2_c[5], rc);
#  else
	printf("mmap(NULL, %lu, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS"
	       ", %d, %#lx) = %#lx\n",
	       args2_c[1], (int) args2_c[4], args2_c[5], rc);
#  endif
# endif

	void *addr = (void *) rc;
	if (mprotect(addr, page_size, PROT_NONE))
		perror_msg_and_fail("mprotect(%p, %lu, PROT_NONE)",
				    addr, page_size);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("defined __NR_mmap "
	"&& (defined __arm__ || defined __i386__ || defined __m68k__ "
		"|| defined __s390__ || defined __s390x__) "
	"&& (defined PATH_TRACING || !(defined __s390x__ || defined __m68k__))")

#endif
