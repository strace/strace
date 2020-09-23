/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_fcntl

# define TEST_SYSCALL_NR __NR_fcntl
# define TEST_SYSCALL_STR "fcntl"
# include "fcntl-common.c"

static void
test_flock64_undecoded(const int cmd, const char *name)
{
	struct_kernel_flock64 fl = {
		.l_type = F_RDLCK,
		.l_start = 0xdefaced1facefeedULL,
		.l_len = 0xdefaced2cafef00dULL
	};
	invoke_test_syscall(0, cmd, &fl);
	pidns_print_leader();
	printf("%s(0, %s, %p) = %s\n",
	       TEST_SYSCALL_STR, name, &fl, errstr);
}

# define TEST_FLOCK64_UNDECODED(cmd) test_flock64_undecoded(cmd, #cmd)

static void
test_flock64_lk64(void)
{
/*
 * F_[GS]ETOWN_EX had conflicting values with F_[GS]ETLK64
 * in kernel revisions v2.6.32-rc1~96..v2.6.32-rc7~23.
 */
# if !defined(F_GETOWN_EX) || F_GETOWN_EX != F_SETLK64
	TEST_FLOCK64_UNDECODED(F_SETLK64);
# endif
/* F_GETLK and F_SETLKW64 have conflicting values on mips64 */
# if !defined(__mips64) || F_GETLK != F_SETLKW64
	TEST_FLOCK64_UNDECODED(F_SETLKW64);
# endif
# if !defined(F_SETOWN_EX) || F_SETOWN_EX != F_GETLK64
	TEST_FLOCK64_UNDECODED(F_GETLK64);
# endif
}

#else

SKIP_MAIN_UNDEFINED("__NR_fcntl")

#endif
