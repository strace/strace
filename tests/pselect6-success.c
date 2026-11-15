/*
 * Check decoding of the descriptor sets reported by pselect6.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_pselect6 || defined __NR_pselect6_time64

# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <sys/select.h>

# ifdef __NR_pselect6
#  define SYSCALL_NR __NR_pselect6
#  define SYSCALL_NAME "pselect6"
# else
#  define SYSCALL_NR __NR_pselect6_time64
#  define SYSCALL_NAME "pselect6_time64"
# endif

static long
k_pselect6(fd_set *const rfds, fd_set *const wfds, fd_set *const efds)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;

	return syscall(SYSCALL_NR, fill | 8, f8ill_ptr_to_kulong(rfds),
		       f8ill_ptr_to_kulong(wfds), f8ill_ptr_to_kulong(efds),
		       0, 0);
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(fd_set, rfds);
	TAIL_ALLOC_OBJECT_CONST_PTR(fd_set, wfds);
	TAIL_ALLOC_OBJECT_CONST_PTR(fd_set, efds);
	long rc;

	FD_ZERO(rfds);
	FD_ZERO(wfds);
	FD_ZERO(efds);
	FD_SET(1, rfds);
	FD_SET(2, efds);
	rc = k_pselect6(rfds, wfds, efds);
	printf("%s(8, [1], [], [2], NULL, NULL) = %ld (in [1], except [2])"
	       " (INJECTED)\n", SYSCALL_NAME, rc);

	FD_ZERO(efds);
	rc = k_pselect6(rfds, wfds, efds);
	printf("%s(8, [1], [], [], NULL, NULL) = %ld (in [1]) (INJECTED)\n",
	       SYSCALL_NAME, rc);

	FD_ZERO(rfds);
	FD_SET(1, wfds);
	FD_SET(2, efds);
	rc = k_pselect6(rfds, wfds, efds);
	printf("%s(8, [], [1], [2], NULL, NULL) = %ld (out [1], except [2])"
	       " (INJECTED)\n", SYSCALL_NAME, rc);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pselect6 || __NR_pselect6_time64")

#endif
