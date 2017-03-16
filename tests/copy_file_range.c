/*
 * This file is part of copy_file_range strace test.
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
#include "scno.h"

#if defined __NR_copy_file_range

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const long int fd_in = (long int) 0xdeadbeefffffffff;
	const long int fd_out = (long int) 0xdeadbeeffffffffe;
	TAIL_ALLOC_OBJECT_CONST_PTR(long long int, off_in);
	TAIL_ALLOC_OBJECT_CONST_PTR(long long int, off_out);
	*off_in = 0xdeadbef1facefed1;
	*off_out = 0xdeadbef2facefed2;
	const size_t len = (size_t) 0xdeadbef3facefed3ULL;
	const unsigned int flags = 0;

	long rc = syscall(__NR_copy_file_range,
			  fd_in, off_in, fd_out, off_out, len, flags);
	printf("copy_file_range(%d, [%lld], %d, [%lld], %zu, %u)"
	       " = %ld %s (%m)\n",
	       (int) fd_in, *off_in, (int) fd_out, *off_out, len, flags,
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_copy_file_range")

#endif
