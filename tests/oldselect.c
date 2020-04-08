/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_select && defined __NR__newselect \
 && __NR_select != __NR__newselect \
 && !defined __sparc__

# define TEST_SYSCALL_NR __NR_select
# define TEST_SYSCALL_STR "select"
# define xselect xselect
# include "xselect.c"

static uint32_t *args;

static long
xselect(const kernel_ulong_t nfds,
	const kernel_ulong_t rs,
	const kernel_ulong_t ws,
	const kernel_ulong_t es,
	const kernel_ulong_t tv)
{
	if (!args)
		args = tail_alloc(sizeof(*args) * 5);
	args[0] = nfds;
	args[1] = rs;
	args[2] = ws;
	args[3] = es;
	args[4] = tv;
	long rc = syscall(TEST_SYSCALL_NR, args);
	errstr = sprintrc(rc);
	return rc;
}

#else

SKIP_MAIN_UNDEFINED("__NR_select && __NR__newselect"
		    " && __NR_select != __NR__newselect"
		    " && !defined __sparc__")

#endif
