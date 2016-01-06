/*
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#if defined(HAVE_PREADV) && defined(HAVE_PWRITEV)

# include <fcntl.h>
# include <unistd.h>
# include <sys/uio.h>
# include <assert.h>

int
main(void)
{
	const off_t offset = 0xdefaceddeadbeefLL;
	char buf[4];
	struct iovec iov = { buf, sizeof buf };

	(void) close(0);
	assert(open("/dev/zero", O_RDONLY) == 0);
	assert(pread(0, buf, sizeof buf, offset) == 4);
	assert(preadv(0, &iov, 1, offset) == 4);
	assert(!close(0));

	assert(open("/dev/null", O_WRONLY) == 0);
	assert(pwrite(0, buf, sizeof buf, offset) == 4);
	assert(pwritev(0, &iov, 1, offset) == 4);
	assert(!close(0));

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_PREADV && HAVE_PWRITEV")

#endif
