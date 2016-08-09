/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#if defined __NR_select && defined __NR__newselect \
 && __NR_select != __NR__newselect \
 && !defined SPARC

# include <unistd.h>
# include <sys/select.h>

int
main(void)
{
	int fds[2];
	fd_set r = {}, w = {};
	struct timeval timeout = { .tv_sec = 0, .tv_usec = 42 };
	long args[] = {
		2, (long) &r, (long) &w, 0, (long) &timeout,
		0xdeadbeef, 0xbadc0ded, 0xdeadbeef, 0xbadc0ded, 0xdeadbeef
	};

	(void) close(0);
	(void) close(1);
	if (pipe(fds))
		perror_msg_and_fail("pipe");

	FD_SET(0, &w);
	FD_SET(1, &r);
	if (syscall(__NR_select, args))
		perror_msg_and_skip("select");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_select && __NR__newselect")

#endif
