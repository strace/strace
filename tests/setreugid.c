/*
 * Check decoding of setreuid/setregid/setreuid32/setregid32 syscalls.
 *
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

static void
print_int(const unsigned int num)
{
	if (num == -1U)
		printf("-1");
	else
		printf("%u", num);
}

static int
num_matches_id(const unsigned int num, const unsigned int ugid)
{
	return num == ugid || num == -1U;
}

#define PAIR(val)	{ val, ugid }, { ugid, val }

int
main(void)
{
	unsigned int ugid = GETUGID;
	CHECK_OVERFLOWUGID(ugid);

	const struct {
		const long r, e;
	} tests[] = {
		{ ugid, ugid },
		PAIR((unsigned long) 0xffffffff00000000ULL | ugid),
		PAIR(-1U),
		PAIR(-1L),
		PAIR(0xffff0000U | ugid),
		PAIR(0xffff),
		PAIR(0xc0deffffU)
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(tests); ++i) {
		const unsigned int rn = ugid2int(tests[i].r);
		const unsigned int en = ugid2int(tests[i].e);

		if (!num_matches_id(rn, ugid) || !num_matches_id(en, ugid))
			continue;

		if (syscall(SYSCALL_NR, tests[i].r, tests[i].e)) {
			if (!i && ENOSYS == errno) {
				printf("%s(%u, %u) = -1 ENOSYS (%m)\n",
				       SYSCALL_NAME, ugid, ugid);
				break;
			}
			perror_msg_and_fail("%s(%#lx, %#lx)", SYSCALL_NAME,
					    tests[i].r, tests[i].e);
		}

		printf("%s(", SYSCALL_NAME);
		print_int(rn);
		printf(", ");
		print_int(en);
		printf(") = 0\n");
	}

	puts("+++ exited with 0 +++");
	return 0;
}
