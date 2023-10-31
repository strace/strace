/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_truncate64

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char fname[] = "truncate64\nfilename";
	static const char qname[] = "truncate64\\nfilename";
	const off_t len = 0xdefaceddeadbeefULL;

	int rc = truncate(fname, len);
	printf("truncate64(\"%s\", %llu) = %s\n",
	       qname, (unsigned long long) len, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_truncate64")

#endif
