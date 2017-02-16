/*
 * Check decoding of setfsuid/setfsgid/setfsuid32/setfsgid32 syscalls.
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

#include <stdio.h>
#include <unistd.h>

void
printuid(unsigned UGID_TYPE id)
{
	if (id == (unsigned UGID_TYPE) -1U)
		printf("-1");
	else
		printf("%u", id);
}

int
main(void)
{
	unsigned int ugid = GETUGID;

	const kernel_ulong_t tests[] = {
		ugid,
		0xffff0000U | ugid,
		F8ILL_KULONG_MASK | ugid,
		0xffffU,
		-1U,
		-1L,
		0xc0deffffU,
		0xfacefeedU,
		(long) 0xfacefeeddeadbeefULL
	};

	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		const unsigned int num = (unsigned UGID_TYPE) tests[i];
		unsigned int rc;

		rc = syscall(SYSCALL_NR, tests[i]);
		printf("%s(", SYSCALL_NAME);
		printuid(num);
		printf(") = %u\n", rc);

		rc = syscall(SYSCALL_NR, ugid);
		printf("%s(%u) = %u\n", SYSCALL_NAME, ugid, rc);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
