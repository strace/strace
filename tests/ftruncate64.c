/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_ftruncate64

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const off_t len = 0xdefaceddeadbeefULL;

	int rc = ftruncate(-1, len);
	printf("ftruncate64(-1, %llu) = %d %s (%m)\n",
	       (unsigned long long) len, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_ftruncate64")

#endif
