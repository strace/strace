/*
 * Check decoding of pipe2 syscall.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef __NR_pipe2

# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	int *const fds = tail_alloc(sizeof(*fds) * 2);
	int *const efault = fds + 1;
	long rc;

	rc = syscall(__NR_pipe2, fds, F8ILL_KULONG_MASK | O_NONBLOCK);
	if (rc)
		perror_msg_and_skip("pipe2");
	printf("pipe2([%d, %d], O_NONBLOCK) = 0\n", fds[0], fds[1]);

	rc = syscall(__NR_pipe2, efault, F8ILL_KULONG_MASK);
	printf("pipe2(%p, 0) = %s\n", efault, sprintrc(rc));

	if (F8ILL_KULONG_SUPPORTED) {
		const kernel_ulong_t ill = f8ill_ptr_to_kulong(fds);
		rc = syscall(__NR_pipe2, ill, 0);
		printf("pipe2(%#llx, 0) = %s\n",
		       (unsigned long long) ill, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pipe2")

#endif
