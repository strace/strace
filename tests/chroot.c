/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_chroot

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample[] = "chroot.sample";

	long rc = syscall(__NR_chroot, sample);
	printf("chroot(\"%s\") = %ld %s (%m)\n",
	       sample, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_chroot")

#endif
