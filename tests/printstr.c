/*
 * Check decoding of non-NUL-terminated strings when len == -1.
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>

#define DEFAULT_STRLEN 32

int
main(void)
{
	char *const buf = tail_alloc(DEFAULT_STRLEN + 1);
	const struct iovec io = {
		.iov_base = buf,
		.iov_len = -1
	};
	int rc;

	buf[0] = 0;

	tprintf("%s", "");

	memset(buf + 1, 'X', DEFAULT_STRLEN);
	buf[DEFAULT_STRLEN - 1] = 0;

	rc = writev(-1, &io, 1);
	tprintf("writev(-1, [{iov_base=\"\\0%*s\\0\"..., iov_len=%lu}], 1)"
		" = %s\n", DEFAULT_STRLEN - 2, buf + 1, -1UL, sprintrc(rc));

	buf[DEFAULT_STRLEN - 1] = 'X';
	buf[DEFAULT_STRLEN] = 0;

	rc = writev(-1, &io, 1);
	tprintf("writev(-1, [{iov_base=\"\\0%*s\"..., iov_len=%lu}], 1)"
		" = %s\n", DEFAULT_STRLEN - 1, buf + 1, -1UL, sprintrc(rc));

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
