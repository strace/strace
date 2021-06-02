/*
 * Check decoding of getgroups/getgroups32 syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifdef __NR_getgroups32

# define SYSCALL_NR	__NR_getgroups32
# define SYSCALL_NAME	"getgroups32"
# define GID_TYPE	unsigned int

#else /* __NR_getgroups */

# include "tests.h"
# include "scno.h"

# define SYSCALL_NR	__NR_getgroups
# define SYSCALL_NAME	"getgroups"
# if defined __NR_getgroups32 && __NR_getgroups != __NR_getgroups32
#  define GID_TYPE	unsigned short
# else
#  define GID_TYPE	unsigned int
# endif

#endif

#include <stdio.h>
#include <unistd.h>

#define MAX_STRLEN 32
static long ngroups;

static void
get_groups(const long size, GID_TYPE *const g)
{
	long i = syscall(SYSCALL_NR, size, g);
	if (i != ngroups)
		perror_msg_and_fail("%s(%#lx, %p)", SYSCALL_NAME, size, g);

	printf("%s(%d, [", SYSCALL_NAME, (int) size);
	for (i = 0; i < ngroups; ++i) {
		if (i)
			printf(", ");
		if (i >= MAX_STRLEN) {
			printf("...");
			break;
		}
		printf("%u", (unsigned int) g[i]);
	}
	printf("]) = %ld\n", ngroups);
}

int
main(void)
{
	long rc;

	/* check how the first argument is decoded */
	ngroups = syscall(SYSCALL_NR, 0, 0);
	printf("%s(0, NULL) = %ld\n", SYSCALL_NAME, ngroups);
	if (ngroups < 0)
		perror_msg_and_fail(SYSCALL_NAME);

	rc = syscall(SYSCALL_NR, F8ILL_KULONG_MASK, 0);
	printf("%s(0, NULL) = %ld\n", SYSCALL_NAME, rc);

	rc = syscall(SYSCALL_NR, -1U, 0);
	printf("%s(%d, NULL) = %s\n", SYSCALL_NAME, -1, sprintrc(rc));

	rc = syscall(SYSCALL_NR, -1L, 0);
	printf("%s(%d, NULL) = %s\n", SYSCALL_NAME, -1, sprintrc(rc));

	const unsigned int ngroups_max = sysconf(_SC_NGROUPS_MAX);

	rc = syscall(SYSCALL_NR, ngroups_max, 0);
	printf("%s(%d, NULL) = %s\n", SYSCALL_NAME, ngroups_max, sprintrc(rc));

	rc = syscall(SYSCALL_NR, F8ILL_KULONG_MASK | ngroups_max, 0);
	printf("%s(%d, NULL) = %s\n", SYSCALL_NAME, ngroups_max, sprintrc(rc));

	/* check how the second argument is decoded */
	GID_TYPE *const g1 =
		tail_alloc(ngroups ? sizeof(*g1) * ngroups : 1);
	GID_TYPE *const g2 = tail_alloc(sizeof(*g2) * (ngroups + 1));
	void *efault = g2 + ngroups + 1;

	get_groups(ngroups, g1);
	get_groups(ngroups + 1, g1);
	get_groups(ngroups + 1, g2);

	if (ngroups) {
		rc = syscall(SYSCALL_NR, ngroups, efault);
		printf("%s(%d, %p) = %s\n",
		       SYSCALL_NAME, (unsigned) ngroups, efault, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
