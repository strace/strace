/*
 * Check decoding of chown/chown32/lchown/lchown32/fchown/fchown32 syscalls.
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

#define UNLINK_SAMPLE \
	if (unlink(sample)) perror_msg_and_fail("unlink")
#define CLOSE_SAMPLE \
	if (close(fd)) perror_msg_and_fail("close")

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
