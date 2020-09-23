/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_fcntl64

# define TEST_SYSCALL_NR __NR_fcntl64
# define TEST_SYSCALL_STR "fcntl64"
# include "fcntl-common.c"

static void
test_flock64_lk64(void)
{
	TEST_FLOCK64_EINVAL(F_SETLK64);
	TEST_FLOCK64_EINVAL(F_SETLKW64);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct_kernel_flock64, fl);
	memset(fl, 0, sizeof(*fl));
	fl->l_type = F_RDLCK;
	fl->l_len = FILE_LEN;

	long rc = invoke_test_syscall(0, F_SETLK64, fl);
	pidns_print_leader();
	printf("%s(0, F_SETLK64, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = %s\n",
	       TEST_SYSCALL_STR, FILE_LEN, errstr);

	if (rc)
		return;

	invoke_test_syscall(0, F_GETLK64, fl);
	pidns_print_leader();
	printf("%s(0, F_GETLK64, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d, l_pid=0}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);

	invoke_test_syscall(0, F_SETLKW64, fl);
	pidns_print_leader();
	printf("%s(0, F_SETLKW64, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);
}

#else

SKIP_MAIN_UNDEFINED("__NR_fcntl64")

#endif
