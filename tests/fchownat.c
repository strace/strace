/*
 * Check decoding of fchownat syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <fcntl.h>

#if defined __NR_fchownat && defined AT_FDCWD && defined AT_SYMLINK_NOFOLLOW

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample[] = "fchownat_sample";
	uid_t uid = geteuid();
	uid_t gid = getegid();

	if (open(sample, O_RDONLY | O_CREAT, 0400) == -1)
		perror_msg_and_fail("open");

	long rc = syscall(__NR_fchownat, AT_FDCWD, sample, uid, gid, 0);
	printf("fchownat(AT_FDCWD, \"%s\", %d, %d, 0) = %s\n",
	       sample, uid, gid, sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_fchownat, AT_FDCWD,
		     sample, -1, -1L, AT_SYMLINK_NOFOLLOW);
	printf("fchownat(AT_FDCWD, \"%s\", -1, -1, AT_SYMLINK_NOFOLLOW) = %s\n",
	       sample, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fchownat && AT_FDCWD && AT_SYMLINK_NOFOLLOW")

#endif
