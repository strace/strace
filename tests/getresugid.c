/*
 * Check decoding of getresuid/getresgid/getresuid32/getresgid32 syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned UGID_TYPE, r);
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned UGID_TYPE, e);
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned UGID_TYPE, s);

	if (syscall(SYSCALL_NR, r, e, s))
		perror_msg_and_fail(SYSCALL_NAME);

	printf("%s([%u], [%u], [%u]) = 0\n", SYSCALL_NAME,
	       (unsigned) *r, (unsigned) *e, (unsigned) *s);

	assert(syscall(SYSCALL_NR, NULL, e, s) == -1);
	printf("%s(NULL, %p, %p) = -1 EFAULT (%m)\n", SYSCALL_NAME, e, s);

	assert(syscall(SYSCALL_NR, r, NULL, s) == -1);
	printf("%s(%p, NULL, %p) = -1 EFAULT (%m)\n", SYSCALL_NAME, r, s);

	assert(syscall(SYSCALL_NR, r, e, NULL) == -1);
	printf("%s(%p, %p, NULL) = -1 EFAULT (%m)\n", SYSCALL_NAME, r, e);

	puts("+++ exited with 0 +++");
	return 0;
}
