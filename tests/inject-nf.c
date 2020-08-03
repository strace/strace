/*
 * Check decoding of return values injected into a syscall that "never fails".
 *
 * Copyright (c) 2018-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "scno.h"

#include "raw_syscall.h"

#ifdef __NR_geteuid32
# define SC_NR __NR_geteuid32
# define SC_NAME "geteuid32"
#else
# define SC_NR __NR_geteuid
# define SC_NAME "geteuid"
#endif

#ifdef raw_syscall_0
# define INVOKE_SC(err) raw_syscall_0(SC_NR, &err)
#else
/* No raw_syscall_0, let's use geteuid() and hope for the best. */
# define INVOKE_SC(err) geteuid()
#endif

/*
 * This prototype is intentionally different
 * from the prototype provided by <unistd.h>.
 */
extern kernel_ulong_t geteuid(void);

int
main(int ac, char **av)
{
	assert(ac == 1 || ac == 2);

	kernel_ulong_t expected =
		(ac == 1) ? geteuid() : strtoull(av[1], NULL, 0);
	kernel_ulong_t err = 0;
	kernel_ulong_t rc = INVOKE_SC(err);

	if (err || rc != expected)
		error_msg_and_fail("expected %#llx, got rval=%#llx err=%#llx",
				   (unsigned long long) expected,
				   (unsigned long long) rc,
				   (unsigned long long) err);

	if (ac == 2) {
		printf("%s() = %llu (INJECTED)\n",
		       SC_NAME, (unsigned long long) rc);

		puts("+++ exited with 0 +++");
	}

	return 0;
}
