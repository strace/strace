/*
 * Check decoding of setgroups/setgroups32 syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifdef __NR_setgroups32

# define SYSCALL_NR	__NR_setgroups32
# define SYSCALL_NAME	"setgroups32"
# define GID_TYPE	unsigned int

#else

# include "tests.h"
# include "scno.h"

# ifdef __NR_setgroups

#  define SYSCALL_NR	__NR_setgroups
#  define SYSCALL_NAME	"setgroups"
#  if defined __NR_setgroups32 && __NR_setgroups != __NR_setgroups32
#   define GID_TYPE	unsigned short
#  else
#   define GID_TYPE	unsigned int
#  endif

# endif

#endif

#ifdef GID_TYPE

# include <stdio.h>
# include <unistd.h>

void
printuid(GID_TYPE id)
{
	if (id == (GID_TYPE) -1U)
		printf("-1");
	else
		printf("%u", id);
}

int
main(void)
{
	const char *errstr;

	/* check how the first argument is decoded */
	long rc = syscall(SYSCALL_NR, 0, 0);
	printf("%s(0, NULL) = %s\n", SYSCALL_NAME, sprintrc(rc));

	rc = syscall(SYSCALL_NR, F8ILL_KULONG_MASK, 0);
	printf("%s(0, NULL) = %s\n", SYSCALL_NAME, sprintrc(rc));

	rc = syscall(SYSCALL_NR, 1, 0);
	printf("%s(1, NULL) = %s\n", SYSCALL_NAME, sprintrc(rc));

	rc = syscall(SYSCALL_NR, (long) 0xffffffff00000001ULL, 0);
	printf("%s(1, NULL) = %s\n", SYSCALL_NAME, sprintrc(rc));

	rc = syscall(SYSCALL_NR, -1U, 0);
	printf("%s(%d, NULL) = %s\n", SYSCALL_NAME, -1, sprintrc(rc));

	rc = syscall(SYSCALL_NR, -1L, 0);
	printf("%s(%d, NULL) = %s\n", SYSCALL_NAME, -1, sprintrc(rc));

	/* check how the second argument is decoded */
	TAIL_ALLOC_OBJECT_CONST_PTR(const GID_TYPE, g1);
	GID_TYPE *const g2 = tail_alloc(sizeof(*g2) * 2);
	GID_TYPE *const g3 = tail_alloc(sizeof(*g3) * 3);

	rc = syscall(SYSCALL_NR, 0, g1 + 1);
	printf("%s(0, []) = %s\n", SYSCALL_NAME, sprintrc(rc));

	rc = syscall(SYSCALL_NR, 1, g1);
	errstr = sprintrc(rc);
	printf("%s(1, [", SYSCALL_NAME);
	printuid(*g1);
	printf("]) = %s\n", errstr);

	rc = syscall(SYSCALL_NR, 1, g1 + 1);
	printf("%s(1, %p) = %s\n", SYSCALL_NAME, g1 + 1, sprintrc(rc));

	rc = syscall(SYSCALL_NR, 1, -1L);
	printf("%s(1, %#lx) = %s\n", SYSCALL_NAME, -1L, sprintrc(rc));

	rc = syscall(SYSCALL_NR, 2, g1);
	errstr = sprintrc(rc);
	printf("%s(2, [", SYSCALL_NAME);
	printuid(*g1);
	printf(", ... /* %p */]) = %s\n", g1 + 1, errstr);

	g2[0] = -2;
	g2[1] = -3;
	rc = syscall(SYSCALL_NR, 2, g2);
	errstr = sprintrc(rc);
	printf("%s(2, [", SYSCALL_NAME);
	printuid(g2[0]);
	printf(", ");
	printuid(g2[1]);
	printf("]) = %s\n", errstr);

	rc = syscall(SYSCALL_NR, 3, g2);
	errstr = sprintrc(rc);
	printf("%s(3, [", SYSCALL_NAME);
	printuid(g2[0]);
	printf(", ");
	printuid(g2[1]);
	printf(", ... /* %p */]) = %s\n", g2 + 2, errstr);

	g3[0] = 0;
	g3[1] = 1;
	rc = syscall(SYSCALL_NR, 3, g3);
	errstr = sprintrc(rc);
	printf("%s(3, [", SYSCALL_NAME);
	printuid(g3[0]);
	printf(", ");
	printuid(g3[1]);
	printf(", ...]) = %s\n", errstr);

	rc = syscall(SYSCALL_NR, 4, g3);
	errstr = sprintrc(rc);
	printf("%s(4, [", SYSCALL_NAME);
	printuid(g3[0]);
	printf(", ");
	printuid(g3[1]);
	printf(", ...]) = %s\n", errstr);

	rc = sysconf(_SC_NGROUPS_MAX);
	const unsigned ngroups_max = rc;

	if ((unsigned long) rc == ngroups_max && (int) ngroups_max > 0) {
		rc = syscall(SYSCALL_NR, ngroups_max, g3);
		errstr = sprintrc(rc);
		printf("%s(%d, [", SYSCALL_NAME, ngroups_max);
		printuid(g3[0]);
		printf(", ");
		printuid(g3[1]);
		printf(", ...]) = %s\n", errstr);

		rc = syscall(SYSCALL_NR, F8ILL_KULONG_MASK | ngroups_max, g3);
		errstr = sprintrc(rc);
		printf("%s(%d, [", SYSCALL_NAME, ngroups_max);
		printuid(g3[0]);
		printf(", ");
		printuid(g3[1]);
		printf(", ...]) = %s\n", errstr);

		rc = syscall(SYSCALL_NR, ngroups_max + 1, g3);
		printf("%s(%d, %p) = %s\n", SYSCALL_NAME,
		       ngroups_max + 1, g3, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_setgroups")

#endif
