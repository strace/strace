/*
 * Check decoding of setgroups/setgroups32 syscalls.
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

#ifdef __NR_setgroups32

# define SYSCALL_NR	__NR_setgroups32
# define SYSCALL_NAME	"setgroups32"
# define GID_TYPE	unsigned int

#else

# include "tests.h"
# include <sys/syscall.h>

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

int
main(void)
{
	/* check how the first argument is decoded */
	if (syscall(SYSCALL_NR, 0, 0))
		printf("%s(0, NULL) = -1 %s (%m)\n", SYSCALL_NAME, errno2name());
	else
		printf("%s(0, NULL) = 0\n", SYSCALL_NAME);

	if (syscall(SYSCALL_NR, (long) 0xffffffff00000000ULL, 0))
		printf("%s(0, NULL) = -1 %s (%m)\n",
		       SYSCALL_NAME, errno2name());
	else
		printf("%s(0, NULL) = 0\n", SYSCALL_NAME);

	syscall(SYSCALL_NR, 1, 0);
	printf("%s(1, NULL) = -1 %s (%m)\n", SYSCALL_NAME, errno2name());

	syscall(SYSCALL_NR, (long) 0xffffffff00000001ULL, 0);
	printf("%s(1, NULL) = -1 %s (%m)\n", SYSCALL_NAME, errno2name());

	syscall(SYSCALL_NR, -1U, 0);
	printf("%s(%u, NULL) = -1 %s (%m)\n", SYSCALL_NAME, -1U, errno2name());

	syscall(SYSCALL_NR, -1L, 0);
	printf("%s(%u, NULL) = -1 %s (%m)\n", SYSCALL_NAME, -1U, errno2name());

	/* check how the second argument is decoded */
	const GID_TYPE *const g1 = tail_alloc(sizeof(*g1));
	GID_TYPE *const g2 = tail_alloc(sizeof(*g2) * 2);
	GID_TYPE *const g3 = tail_alloc(sizeof(*g3) * 3);

	if (syscall(SYSCALL_NR, 0, g1 + 1))
		printf("%s(0, []) = -1 %s (%m)\n",
		       SYSCALL_NAME, errno2name());
	else
		printf("%s(0, []) = 0\n", SYSCALL_NAME);

	if (syscall(SYSCALL_NR, 1, g1))
		printf("%s(1, [%u]) = -1 %s (%m)\n",
		       SYSCALL_NAME, (unsigned) *g1, errno2name());
	else
		printf("%s(1, [%u]) = 0\n",
		       SYSCALL_NAME, (unsigned) *g1);

	syscall(SYSCALL_NR, 1, g1 + 1);
	printf("%s(1, %p) = -1 %s (%m)\n",
	       SYSCALL_NAME, g1 + 1, errno2name());

	syscall(SYSCALL_NR, 1, -1L);
	printf("%s(1, %#lx) = -1 %s (%m)\n", SYSCALL_NAME, -1L, errno2name());

	syscall(SYSCALL_NR, 2, g1);
	printf("%s(2, [%u, %p]) = -1 %s (%m)\n",
	       SYSCALL_NAME, (unsigned) *g1, g1 + 1, errno2name());

	g2[0] = -2;
	g2[1] = -3;
	if (syscall(SYSCALL_NR, 2, g2))
		printf("%s(2, [%u, %u]) = -1 %s (%m)\n", SYSCALL_NAME,
		       (unsigned) g2[0], (unsigned) g2[1], errno2name());
	else
		printf("%s(2, [%u, %u]) = 0\n", SYSCALL_NAME,
		       (unsigned) g2[0], (unsigned) g2[1]);

	syscall(SYSCALL_NR, 3, g2);
	printf("%s(3, [%u, %u, %p]) = -1 %s (%m)\n", SYSCALL_NAME,
	       (unsigned) g2[0], (unsigned) g2[1], g2 + 2, errno2name());

	g3[0] = 0;
	g3[1] = 1;
	if (syscall(SYSCALL_NR, 3, g3))
		printf("%s(3, [%u, %u, ...]) = -1 %s (%m)\n", SYSCALL_NAME,
		       (unsigned) g3[0], (unsigned) g3[1], errno2name());
	else
		printf("%s(3, [%u, %u]) = 0\n", SYSCALL_NAME,
		       (unsigned) g3[0], (unsigned) g3[1]);

	syscall(SYSCALL_NR, 4, g3);
	printf("%s(4, [%u, %u, ...]) = -1 %s (%m)\n", SYSCALL_NAME,
	       (unsigned) g3[0], (unsigned) g3[1], errno2name());

	long rc = sysconf(_SC_NGROUPS_MAX);
	const unsigned ngroups_max = rc;

	if ((unsigned long) rc == ngroups_max && (int) ngroups_max > 0) {
		syscall(SYSCALL_NR, ngroups_max, g3);
		printf("%s(%u, [%u, %u, ...]) = -1 %s (%m)\n", SYSCALL_NAME,
		       ngroups_max, (unsigned) g3[0], (unsigned) g3[1],
		       errno2name());

		const unsigned long size =
			(unsigned long) 0xffffffff00000000ULL | ngroups_max;
		syscall(SYSCALL_NR, size, g3);
		printf("%s(%u, [%u, %u, ...]) = -1 %s (%m)\n", SYSCALL_NAME,
		       ngroups_max, (unsigned) g3[0], (unsigned) g3[1],
		       errno2name());

		syscall(SYSCALL_NR, ngroups_max + 1, g3);
		printf("%s(%u, %p) = -1 %s (%m)\n", SYSCALL_NAME,
		       ngroups_max + 1, g3, errno2name());
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_setgroups")

#endif
