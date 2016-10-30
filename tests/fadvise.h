/*
 * Common definitions for fadvise64 and fadvise64_64 tests.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
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

#ifndef STRACE_TESTS_FADVISE_H
#define STRACE_TESTS_FADVISE_H

# include <limits.h>
# include <stdio.h>
# include <unistd.h>

# include "xlat.h"
# include "xlat/advise.h"

static void do_fadvise(long fd, long long offset, long long llen, long advice);

int
main(void)
{
	static const long bogus_fd = (long) 0xfeedf00dbeeffaceULL;
	static const long long bogus_offset = 0xbadc0dedda7a1057ULL;
	static const long long bogus_len = 0xbadfaceca7b0d1e5ULL;
	static const long bogus_advice = (long) 0xf00dfeeddeadca75ULL;

	do_fadvise(bogus_fd, bogus_offset, bogus_len, bogus_advice);

	puts("+++ exited with 0 +++");
	return 0;
}

#endif /* !STRACE_TESTS_FADVISE_H */
