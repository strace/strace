/*
 * Check decoding of fallocate syscall.
 *
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

#if defined(__NR_fallocate) && defined(HAVE_FALLOCATE) && HAVE_FALLOCATE

# include <errno.h>
# include <fcntl.h>
# include <stdio.h>

# include "xlat.h"
# include "xlat/falloc_flags.h"

int
main(void)
{
	static const int bogus_fd = 0xbeefface;
	static const int bogus_mode = 0xdeadca75;
	static const off_t bogus_offset = (off_t) 0xbadc0dedda7a1057LLU;
	static const off_t bogus_len = (off_t) 0xbadfaceca7b0d1e5LLU;

	long rc = fallocate(bogus_fd, bogus_mode, bogus_offset, bogus_len);
	/*
	 * Workaround a bug fixed by commit glibc-2.11-346-gde240a0.
	 */
	if (rc > 0) {
		errno = rc;
		rc = -1;
	}
	const char *errstr = sprintrc(rc);

	printf("fallocate(%d, ", bogus_fd);
	printflags(falloc_flags, (unsigned) bogus_mode, "FALLOC_FL_???");
	printf(", %lld, %lld) = %s\n",
	       (long long) bogus_offset, (long long) bogus_len, errstr);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fallocate && HAVE_FALLOCATE");

#endif
