/*
 * Copyright (c) 2015-2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include "scno.h"
#include <unistd.h>

#ifdef __NR_oldumount
# define TEST_SYSCALL_STR "oldumount"
#else
# if defined __NR_umount && defined __NR_umount2 && __NR_umount != __NR_umount2
#  define __NR_oldumount __NR_umount
#  define TEST_SYSCALL_STR "umount"
# endif
#endif

#ifdef __NR_oldumount

int
main(void)
{
	static const char sample[] = "umount.sample";
	if (mkdir(sample, 0700))
		perror_msg_and_fail("mkdir: %s", sample);

	long rc = syscall(__NR_oldumount, sample);
	printf("%s(\"%s\") = %ld %s (%m)\n",
	       TEST_SYSCALL_STR, sample, rc, errno2name());

	if (rmdir(sample))
		perror_msg_and_fail("rmdir: %s", sample);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_oldumount")

#endif
