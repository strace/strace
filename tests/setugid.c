/*
 * Check decoding of setuid/setgid/setuid32/setgid32 syscalls.
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

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

static int
ugid2int(const unsigned UGID_TYPE ugid)
{
	if ((unsigned UGID_TYPE) -1U == ugid)
		return -1;
	else
		return ugid;
}

int
main(void)
{
	unsigned int ugid = GETUGID;
	CHECK_OVERFLOWUGID(ugid);

	const long tests[] = {
		ugid,
		0xffff0000U | ugid,
		(unsigned long) 0xffffffff00000000ULL | ugid,
		0xffffU,
		-1U,
		-1L
	};

	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		const unsigned int num = ugid2int(tests[i]);
		long expected;

		if (num == ugid)
			expected = 0;
		else if (num == -1U)
			expected = -1;
		else
			continue;

		const long rc = syscall(SYSCALL_NR, tests[i]);
		int saved_errno = errno;
		if (rc != expected) {
			if (!i && ENOSYS == errno) {
				printf("%s(%u) = -1 ENOSYS (%m)\n",
				       SYSCALL_NAME, ugid);
				break;
			}
			perror_msg_and_fail("%s(%#lx) != %ld",
					    SYSCALL_NAME, tests[i], expected);
		}

		printf("%s(", SYSCALL_NAME);
		if (num == -1U)
			printf("-1");
		else
			printf("%u", num);
		errno = saved_errno;
		if (expected)
			printf(") = -1 %s (%m)\n", errno2name());
		else
			printf(") = 0\n");
	}

	puts("+++ exited with 0 +++");
	return 0;
}
