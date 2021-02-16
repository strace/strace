/*
 * Check decoding of chroot syscall.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	static const char sample[] = "chroot.sample";

	printf("chroot(NULL) = %s\n",
	       sprintrc(syscall(__NR_chroot, NULL)));
	printf("chroot(\"%s\") = %s\n",
	       sample, sprintrc(syscall(__NR_chroot, sample)));

	puts("+++ exited with 0 +++");
	return 0;
}
