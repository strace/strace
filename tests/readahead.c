/*
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
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

#ifdef HAVE_READAHEAD
/* Check for glibc readahead argument passing bugs. */
# ifdef __GLIBC__
/*
 * glibc < 2.8 had an incorrect order of higher and lower parts of offset,
 * see https://sourceware.org/bugzilla/show_bug.cgi?id=5208
 */
#  if !(defined __GLIBC_MINOR__ && \
        (__GLIBC__ << 16) + __GLIBC_MINOR__ >= (2 << 16) + 8)
#   undef HAVE_READAHEAD
#  endif /* glibc < 2.8 */
/*
 * glibc < 2.25 had an incorrect implementation on mips n64,
 * see https://sourceware.org/bugzilla/show_bug.cgi?id=21026
 */
#  if defined LINUX_MIPSN64 && !(defined __GLIBC_MINOR__ && \
        (__GLIBC__ << 16) + __GLIBC_MINOR__ >= (2 << 16) + 25)
#   undef HAVE_READAHEAD
#  endif /* LINUX_MIPSN64 && glibc < 2.25 */
# endif /* __GLIBC__ */
#endif /* HAVE_READAHEAD */

#ifdef HAVE_READAHEAD

# include <fcntl.h>
# include <stdio.h>

static const int fds[] = {
	-0x80000000,
	-100,
	-1,
	0,
	1,
	2,
	0x7fffffff,
};

static const off64_t offsets[] = {
	-0x8000000000000000LL,
	-0x5060708090a0b0c0LL,
	-1LL,
	 0,
	 1,
	 0xbadfaced,
	 0x7fffffffffffffffLL,
};

static const unsigned long counts[] = {
	0UL,
	0xdeadca75,
	(unsigned long) 0xface1e55beeff00dULL,
	(unsigned long) 0xffffffffffffffffULL,
};

int
main(void)
{
	unsigned i;
	unsigned j;
	unsigned k;
	ssize_t rc;

	for (i = 0; i < ARRAY_SIZE(fds); i++)
		for (j = 0; j < ARRAY_SIZE(offsets); j++)
			for (k = 0; k < ARRAY_SIZE(counts); k++) {
				rc = readahead(fds[i], offsets[j], counts[k]);

				printf("readahead(%d, %lld, %lu) = %s\n",
					fds[i], (long long) offsets[j],
					counts[k], sprintrc(rc));
			}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_READAHEAD")

#endif
