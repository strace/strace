/*
 * Check decoding of getresuid/getresgid/getresuid32/getresgid32 syscalls.
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

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned UGID_TYPE, r);
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned UGID_TYPE, e);
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned UGID_TYPE, s);

	if (syscall(SYSCALL_NR, r, e, s))
		perror_msg_and_fail(SYSCALL_NAME);

	printf("%s([%u], [%u], [%u]) = 0\n", SYSCALL_NAME,
	       (unsigned) *r, (unsigned) *e, (unsigned) *s);

	assert(syscall(SYSCALL_NR, NULL, e, s) == -1);
	printf("%s(NULL, %p, %p) = -1 EFAULT (%m)\n", SYSCALL_NAME, e, s);

	assert(syscall(SYSCALL_NR, r, NULL, s) == -1);
	printf("%s(%p, NULL, %p) = -1 EFAULT (%m)\n", SYSCALL_NAME, r, s);

	assert(syscall(SYSCALL_NR, r, e, NULL) == -1);
	printf("%s(%p, %p, NULL) = -1 EFAULT (%m)\n", SYSCALL_NAME, r, e);

	puts("+++ exited with 0 +++");
	return 0;
}
