/*
 * Check decoding of setfsuid/setfsgid/setfsuid32/setfsgid32 syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdio.h>
#include <unistd.h>

static void
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

	for (unsigned int i = 0; i < ARRAY_SIZE(tests); ++i) {
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
