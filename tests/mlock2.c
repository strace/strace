/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_mlock2

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const unsigned long addr = (unsigned long) 0xfacefeeddeadbeefULL;
	const unsigned long len = (unsigned long) 0xcafef00dbadc0dedULL;

	long rc = syscall(__NR_mlock2, addr, len, -1UL);
	printf("mlock2(%#lx, %lu, MLOCK_ONFAULT|0xfffffffe)"
	       " = %ld %s (%m)\n", addr, len, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mlock2")

#endif
