/*
 * Check decoding of memfd_secret syscall.
 *
 * Copyright (c) 2021-2024 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "kernel_fcntl.h"
#include "scno.h"

#include <inttypes.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#ifndef RETVAL_INJECTED
# define RETVAL_INJECTED 0
#endif

#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif
#ifndef FD_PATH
# define FD_PATH ""
#endif

#if RETVAL_INJECTED
# define INJ_STR " (INJECTED)\n"
# define INJ_FD_STR FD_PATH " (INJECTED)\n"
#else /* !RETVAL_INJECTED */
# define INJ_STR "\n"
# define INJ_FD_STR "\n"
#endif /* RETVAL_INJECTED */

static const char *errstr;


static long
sys_memfd_secret(unsigned int flags)

{
	static const kernel_ulong_t fill =
		(kernel_ulong_t) 0xbeefcafe00000000ULL;
	kernel_ulong_t arg1 = fill | flags;
	kernel_ulong_t arg2 = fill | 0xdefaeced;
	kernel_ulong_t arg3 = fill | 0xbabefeed;
	kernel_ulong_t arg4 = fill | 0xbadbeefd;
	kernel_ulong_t arg5 = fill | 0xdecaffed;
	kernel_ulong_t arg6 = fill | 0xdeefaced;

	long rc = syscall(__NR_memfd_secret,
			  arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);

	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	static const struct strval32 flags_vals[] = {
		{ ARG_STR(0) },
		{ ARG_STR(O_CLOEXEC) },
		/* O_CLOEXEC is either 0x80000, 0x200000, or 0x400000 */
		{ ARG_STR(0xde97face) },
		{ ARG_STR(O_CLOEXEC|0xde97face) },
	};
	for (size_t i = 0; i < ARRAY_SIZE(flags_vals); i++) {
		long rc = sys_memfd_secret(flags_vals[i].val);
		printf("memfd_secret(%s) = %s%s" INJ_STR,
		       flags_vals[i].str, errstr, rc > 0 ? FD_PATH : "");
	}

	puts("+++ exited with 0 +++");
	return 0;
}
