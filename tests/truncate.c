/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_truncate

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char fname[] = "truncate\nfilename";
	static const char qname[] = "truncate\\nfilename";
	const kernel_ulong_t len = (kernel_ulong_t) 0xdefaced0badc0deULL;
	long rc;

	if (sizeof(len) > sizeof(long))
		rc = truncate(fname, len);
	else
		rc = syscall(__NR_truncate, fname, len);

	printf("truncate(\"%s\", %llu) = %s\n",
	       qname, (unsigned long long) len, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_truncate")

#endif
