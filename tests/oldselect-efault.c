/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2018-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_select && defined __NR__newselect \
 && __NR_select != __NR__newselect \
 && !defined __sparc__

# include <stdint.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <sys/select.h>

static const char *errstr;

static long
xselect(const kernel_ulong_t args)
{
	static const kernel_ulong_t dummy = F8ILL_KULONG_MASK | 0xfacefeed;
	long rc = syscall(__NR_select, args, dummy, dummy, dummy, dummy, dummy);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_ARR(unsigned long, args, 4);
	memset(args, 0, sizeof(*args) * 4);

	xselect(0);
# ifndef PATH_TRACING_FD
	printf("select(NULL) = %s\n", errstr);
# endif

	xselect((uintptr_t) args);
# ifndef PATH_TRACING_FD
	printf("select(%p) = %s\n", args, errstr);
# endif

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_select && __NR__newselect"
		    " && __NR_select != __NR__newselect"
		    " && !defined __sparc__")

#endif
