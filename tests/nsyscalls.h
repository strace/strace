/*
 * Helper header for out-of-range syscalls decoding checks.
 *
 * Copyright (c) 2015-2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "sysent.h"
#include "scno.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sysent_shorthand_defs.h"

static const struct_sysent syscallent[] = {
#include "syscallent.h"
};

#include "sysent_shorthand_undefs.h"

static const kernel_ulong_t out_of_range_syscall_args[] = {
	(kernel_ulong_t) 0xface0fedbadc0dedULL,
	(kernel_ulong_t) 0xface1fedbadc1dedULL,
	(kernel_ulong_t) 0xface2fedbadc2dedULL,
	(kernel_ulong_t) 0xface3fedbadc3dedULL,
	(kernel_ulong_t) 0xface4fedbadc4dedULL,
	(kernel_ulong_t) 0xface5fedbadc5dedULL
};

static long
invoke_syscall(const unsigned long nr)
{
	return syscall(nr | SYSCALL_BIT,
		       out_of_range_syscall_args[0],
		       out_of_range_syscall_args[1],
		       out_of_range_syscall_args[2],
		       out_of_range_syscall_args[3],
		       out_of_range_syscall_args[4],
		       out_of_range_syscall_args[5]);
}
