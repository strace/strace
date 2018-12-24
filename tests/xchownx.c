/*
 * Check decoding of chown/chown32/lchown/lchown32/fchown/fchown32 syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#ifdef UGID_TYPE_IS_SHORT
# define UGID_TYPE	short
# define GETEUID	syscall(__NR_geteuid)
# define GETEGID	syscall(__NR_getegid)
# define CHECK_OVERFLOWUID(arg)	check_overflowuid(arg)
# define CHECK_OVERFLOWGID(arg)	check_overflowgid(arg)
#else
# define UGID_TYPE	int
# define GETEUID	geteuid()
# define GETEGID	getegid()
# define CHECK_OVERFLOWUID(arg)
# define CHECK_OVERFLOWGID(arg)
#endif

#define UNLINK_SAMPLE					\
	do {						\
		if (unlink(sample))			\
			perror_msg_and_fail("unlink");	\
	} while (0)

#define CLOSE_SAMPLE					\
	do {						\
		if (close(fd))				\
			perror_msg_and_fail("close");	\
	} while (0)

#ifdef ACCESS_BY_DESCRIPTOR
# define SYSCALL_ARG1 fd
# define FMT_ARG1 "%d"
# define EOK_CMD CLOSE_SAMPLE
# define CLEANUP_CMD UNLINK_SAMPLE
#else
# define SYSCALL_ARG1 sample
# define FMT_ARG1 "\"%s\""
# define EOK_CMD UNLINK_SAMPLE
# define CLEANUP_CMD CLOSE_SAMPLE
#endif

static int
ugid2int(const unsigned UGID_TYPE id)
{
	if ((unsigned UGID_TYPE) -1U == id)
		return -1;
	else
		return id;
}

static void
print_int(const unsigned int num)
{
	if (num == -1U)
		printf(", -1");
	else
		printf(", %u", num);
}

static int
num_matches_id(const unsigned int num, const unsigned int id)
{
	return num == id || num == -1U;
}

#define PAIR(val)	{ val, gid }, { uid, val }

int
main(void)
{
	static const char sample[] = SYSCALL_NAME "_sample";

	unsigned int uid = GETEUID;
	CHECK_OVERFLOWUID(uid);
	unsigned int gid = GETEGID;
	CHECK_OVERFLOWUID(gid);

	const struct {
		const long uid, gid;
	} tests[] = {
		{ uid, gid },
		{ (unsigned long) 0xffffffff00000000ULL | uid, gid },
		{ uid, (unsigned long) 0xffffffff00000000ULL | gid },
		PAIR(-1U),
		PAIR(-1L),
		{ 0xffff0000U | uid, gid },
		{ uid, 0xffff0000U | gid },
		PAIR(0xffff),
		PAIR(0xc0deffffU),
		PAIR(0xfacefeedU),
		PAIR((long) 0xfacefeeddeadbeefULL)
	};

	int fd = open(sample, O_RDONLY | O_CREAT, 0400);
	if (fd < 0)
		perror_msg_and_fail("open");

	CLEANUP_CMD;

	unsigned int i;
	long expected = 0;

	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		const unsigned int unum = ugid2int(tests[i].uid);
		const unsigned int gnum = ugid2int(tests[i].gid);

		if (num_matches_id(unum, uid) &&
		    num_matches_id(gnum, gid)) {
			if (expected)
				continue;
		} else {
			if (!expected) {
				expected = -1;
				EOK_CMD;
			}
		}

		const long rc = syscall(SYSCALL_NR, SYSCALL_ARG1,
					tests[i].uid, tests[i].gid);
		const char *errstr = sprintrc(rc);
		printf("%s(" FMT_ARG1, SYSCALL_NAME, SYSCALL_ARG1);
		print_int(unum);
		print_int(gnum);
		printf(") = %s\n", errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
