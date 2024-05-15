/*
 * Check decoding of seccomp SECCOMP_GET_NOTIF_SIZES.
 *
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <linux/seccomp.h>

#ifndef SECCOMP_GET_NOTIF_SIZES
# define SECCOMP_GET_NOTIF_SIZES 3
#endif

#ifndef INJECT_RETVAL
# define INJECT_RETVAL 0
#endif

#if INJECT_RETVAL
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif

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
	TAIL_ALLOC_OBJECT_CONST_ARR(uint16_t, sizes, 3);
	kernel_ulong_t op = (kernel_ulong_t) 0xfacefeed00000000ULL
				| SECCOMP_GET_NOTIF_SIZES;
	kernel_ulong_t flags = (kernel_ulong_t) 0xdeadbeef00000000ULL;
	long rc;

	rc = k_seccomp(op, flags | 0xdeadbeef, 0);
	printf("seccomp(SECCOMP_GET_NOTIF_SIZES, 0xdeadbeef, NULL) = %s"
	       INJ_STR "\n", errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		rc = k_seccomp(op, flags, f8ill_ptr_to_kulong(0));
		printf("seccomp(SECCOMP_GET_NOTIF_SIZES, 0, %#llx) = %s"
		       INJ_STR "\n",
		       (unsigned long long) f8ill_ptr_to_kulong(0), errstr);
	}

	rc = k_seccomp(op, flags, (uintptr_t) (sizes + 1));
	printf("seccomp(SECCOMP_GET_NOTIF_SIZES, 0, %p) = %s" INJ_STR "\n",
	       sizes + 1, errstr);

	for (size_t i = 0; i < 2; i++) {
		sizes[0] = 0xcafe;
		sizes[1] = 0xfeed;
		sizes[2] = 0xbeef;
		rc = k_seccomp(op, flags | (i * 0xdeadc0de), (uintptr_t) sizes);
		if (rc < 0
		    && errno != ENOSYS && errno != EINVAL && errno != EPERM) {
			perror_msg_and_fail("Unexpected seccomp("
					    "SECCOMP_GET_NOTIF_SIZES) error");
		}
		printf("seccomp(SECCOMP_GET_NOTIF_SIZES, %s, ",
		       i ? "0xdeadc0de" : "0");
		if (rc >= 0) {
			printf("{seccomp_notif=%hu, seccomp_notif_resp=%hu"
			       ", seccomp_data=%hu}",
			       (uint16_t) (INJECT_RETVAL ? 0xcafe : sizes[0]),
			       (uint16_t) (INJECT_RETVAL ? 0xfeed : sizes[1]),
			       (uint16_t) (INJECT_RETVAL ? 0xbeef : sizes[2]));
		} else {
			printf("%p", sizes);
		}

		printf(") = %s" INJ_STR "\n", errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
