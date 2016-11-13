/*
 * Check decoding of sync_file_range syscall.
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
#include <fcntl.h>
#include <asm/unistd.h>

#if defined HAVE_SYNC_FILE_RANGE && defined __NR_sync_file_range

# include <stdio.h>

int
main(void)
{
	const int fd = -1;
	const off64_t offset = 0xdeadbeefbadc0dedULL;
	const off64_t nbytes = 0xfacefeedcafef00dULL;
	const unsigned int flags = -1;

	int rc = sync_file_range(fd, offset, nbytes, flags);
	printf("%s(%d, %lld, %lld, SYNC_FILE_RANGE_WAIT_BEFORE"
	       "|SYNC_FILE_RANGE_WRITE|SYNC_FILE_RANGE_WAIT_AFTER"
	       "|0xfffffff8) = %d %s (%m)\n",
	       "sync_file_range", fd,
	       (long long) offset,
	       (long long) nbytes,
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYNC_FILE_RANGE && __NR_sync_file_range")

#endif
