/*
 * Copyright (c) 2016 Anchit Jain <anchitjain1234@gmail.com>
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_chmod

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "selinux.c"

int
main(void)
{
	static const char fname[] = "chmod_test_file";
	char *my_secontext = SELINUX_MYCONTEXT();

	unlink(fname);
	if (open(fname, O_CREAT|O_RDONLY, 0400) < 0)
		perror_msg_and_fail("open");

	long rc = syscall(__NR_chmod, fname, 0600);
	printf("%schmod(\"%s\"%s, 0600) = %s\n",
	       my_secontext,
	       fname, SELINUX_FILECONTEXT(fname),
	       sprintrc(rc));

	if (unlink(fname))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_chmod, fname, 051);
	printf("%schmod(\"%s\", 051) = %s\n",
	       my_secontext,
	       fname,
	       sprintrc(rc));

	rc = syscall(__NR_chmod, fname, 004);
	printf("%schmod(\"%s\", 004) = %s\n",
	       my_secontext,
	       fname,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_chmod")

#endif
