/*
 * Check decoding of seccomp SECCOMP_GET_ACTION_AVAIL.
 *
 * Copyright (c) 2017-2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_seccomp

# include <stdio.h>
# include <stdint.h>
# include <unistd.h>

# ifdef HAVE_LINUX_SECCOMP_H
#  include <linux/seccomp.h>
# endif

# ifndef SECCOMP_GET_ACTION_AVAIL
#  define SECCOMP_GET_ACTION_AVAIL 2
# endif

static const char *errstr;

static long
k_seccomp(const kernel_ulong_t op, const kernel_ulong_t flags,
	  const kernel_ulong_t args)
{
	const long rc = syscall(__NR_seccomp, op, flags, args);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, act);
	kernel_ulong_t op = (kernel_ulong_t) 0xfacefeed00000000ULL
				| SECCOMP_GET_ACTION_AVAIL;
	kernel_ulong_t flags = (kernel_ulong_t) 0xdeadbeef00000000ULL;
	unsigned int i;

	struct {
		uint32_t val;
		const char *str;
	} actions [] = {
		{ 0, "SECCOMP_RET_KILL_THREAD" },
# ifdef SECCOMP_RET_KILL_PROCESS
		{ ARG_STR(SECCOMP_RET_KILL_PROCESS) },
# endif
# ifdef SECCOMP_RET_TRAP
		{ ARG_STR(SECCOMP_RET_TRAP) },
# endif
# ifdef SECCOMP_RET_ERRNO
		{ ARG_STR(SECCOMP_RET_ERRNO) },
# endif
# ifdef SECCOMP_RET_USER_NOTIF
		{ ARG_STR(SECCOMP_RET_USER_NOTIF) },
# endif
# ifdef SECCOMP_RET_TRACE
		{ ARG_STR(SECCOMP_RET_TRACE) },
# endif
# ifdef SECCOMP_RET_LOG
		{ ARG_STR(SECCOMP_RET_LOG) },
# endif
# ifdef SECCOMP_RET_ALLOW
		{ ARG_STR(SECCOMP_RET_ALLOW) },
# endif
		{ 0xffffffff, "0xffffffff /* SECCOMP_RET_??? */" }
	};

	for (i = 0; i < ARRAY_SIZE(actions); ++i) {
		*act = actions[i].val;
		k_seccomp(op, flags, (uintptr_t) act);
		printf("seccomp(SECCOMP_GET_ACTION_AVAIL, 0, [%s]) = %s\n",
		       actions[i].str, errstr);
	}

	*act = actions[0].val;

	k_seccomp(op, flags, (uintptr_t) (act + 1));
	printf("seccomp(SECCOMP_GET_ACTION_AVAIL, 0, %p) = %s\n",
	       act + 1, errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		k_seccomp(op, flags, f8ill_ptr_to_kulong(act));
		printf("seccomp(SECCOMP_GET_ACTION_AVAIL, 0, %#jx) = %s\n",
		       (uintmax_t) f8ill_ptr_to_kulong(act), errstr);
	}

	flags |= 0xcafef00d;
	k_seccomp(op, flags, 0);
	printf("seccomp(SECCOMP_GET_ACTION_AVAIL, %u, NULL) = %s\n",
	       (unsigned int) flags, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_seccomp")

#endif
