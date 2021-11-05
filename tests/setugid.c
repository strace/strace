/*
 * Check decoding of setuid/setgid/setuid32/setgid32 syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <errno.h>
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
	CHECK_OVERFLOWUGID(ugid);

	const long tests[] = {
		ugid,
		0xffff0000U | ugid,
		(unsigned long) 0xffffffff00000000ULL | ugid,
		0xffffU,
		-1U,
		-1L
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(tests); ++i) {
		const unsigned int num = (unsigned UGID_TYPE) tests[i];
		long expected;

		errno = 0;

		if (num == ugid)
			expected = 0;
		else if ((UGID_TYPE) num == (UGID_TYPE) -1U)
			expected = -1;
		else
			continue;

		const long rc = syscall(SYSCALL_NR, tests[i]);
		const char *errstr = sprintrc(rc);

		if (rc != expected) {
			if (!i && ENOSYS == errno) {
				printf("%s(%u) = %s\n",
				       SYSCALL_NAME, ugid, errstr);
				break;
			}
			perror_msg_and_fail("%s(%#lx) = %ld != %ld",
					    SYSCALL_NAME, tests[i],
					    rc, expected);
		}

		printf("%s(", SYSCALL_NAME);
		printuid(num);
		printf(") = %s\n", errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
