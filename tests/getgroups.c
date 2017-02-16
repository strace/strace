/*
 * Check decoding of getgroups/getgroups32 syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __NR_getgroups32

# define SYSCALL_NR	__NR_getgroups32
# define SYSCALL_NAME	"getgroups32"
# define GID_TYPE	unsigned int

#else

# include "tests.h"
# include <asm/unistd.h>

# ifdef __NR_getgroups

#  define SYSCALL_NR	__NR_getgroups
#  define SYSCALL_NAME	"getgroups"
#  if defined __NR_getgroups32 && __NR_getgroups != __NR_getgroups32
#   define GID_TYPE	unsigned short
#  else
#   define GID_TYPE	unsigned int
#  endif

# endif

#endif

#ifdef GID_TYPE

# include <stdio.h>
# include <unistd.h>

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

#else

SKIP_MAIN_UNDEFINED("__NR_getgroups")

#endif
