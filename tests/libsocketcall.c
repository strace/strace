/*
 * Invoke a socket syscall, either directly or via __NR_socketcall.
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
#include <errno.h>
#include <unistd.h>
#include <asm/unistd.h>

/*
 * Invoke a socket syscall, either directly or via __NR_socketcall.
 * if nr == -1, no direct syscall invocation will be made.
 */
int
socketcall(const int nr, const int call,
	   long a1, long a2, long a3, long a4, long a5)
{
	int rc = -1;
	errno = ENOSYS;

# ifdef __NR_socketcall
	static int have_socketcall = -1;

	if (have_socketcall < 0) {
		if (syscall(__NR_socketcall, 0L, 0L, 0L, 0L, 0L) < 0
		    && EINVAL == errno) {
			have_socketcall = 1;
		} else {
			have_socketcall = 0;
		}
	}

	if (have_socketcall) {
		const long args[] = { a1, a2, a3, a4, a5 };
		rc = syscall(__NR_socketcall, call, args);
	} else
# endif
	{
		if (nr != -1)
			rc = syscall(nr, a1, a2, a3, a4, a5);
	}

	return rc;
}
