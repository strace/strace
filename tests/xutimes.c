/*
 * Check decoding of utimes/osf_utimes syscall.
 *
 * Copyright (c) 2015-2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TEST_SYSCALL_NR
# error TEST_SYSCALL_NR must be defined
#endif

#ifndef TEST_SYSCALL_STR
# error TEST_SYSCALL_STR must be defined
#endif

#ifndef TEST_STRUCT
# error TEST_STRUCT must be defined
#endif

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

static void
print_tv(const TEST_STRUCT *const tv)
{
	printf("{tv_sec=%lld, tv_usec=%llu}",
	       (long long) tv->tv_sec,
	       zero_extend_signed_to_ull(tv->tv_usec));
	print_time_t_usec(tv->tv_sec,
			  zero_extend_signed_to_ull(tv->tv_usec), 1);
}

static const char *errstr;

static long
k_utimes(const kernel_ulong_t pathname, const kernel_ulong_t times)
{
	long rc = syscall(TEST_SYSCALL_NR, pathname, times);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const char proto_fname[] = TEST_SYSCALL_STR "_sample";
	static const char qname[] = "\"" TEST_SYSCALL_STR "_sample\"";

	char *const fname = tail_memdup(proto_fname, sizeof(proto_fname));
	const kernel_ulong_t kfname = (uintptr_t) fname;
	TAIL_ALLOC_OBJECT_CONST_ARR(TEST_STRUCT, tv, 2);

	/* pathname */
	k_utimes(0, 0);
	printf("%s(NULL, NULL) = %s\n", TEST_SYSCALL_STR, errstr);

	k_utimes(kfname + sizeof(proto_fname) - 1, 0);
	printf("%s(\"\", NULL) = %s\n", TEST_SYSCALL_STR, errstr);

	k_utimes(kfname, 0);
	printf("%s(%s, NULL) = %s\n", TEST_SYSCALL_STR, qname, errstr);

	fname[sizeof(proto_fname) - 1] = '+';
	k_utimes(kfname, 0);
	fname[sizeof(proto_fname) - 1] = '\0';
	printf("%s(%p, NULL) = %s\n", TEST_SYSCALL_STR, fname, errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		k_utimes(f8ill_ptr_to_kulong(fname), 0);
		printf("%s(%#jx, NULL) = %s\n", TEST_SYSCALL_STR,
		       (uintmax_t) f8ill_ptr_to_kulong(fname), errstr);
	}

	/* times */
	k_utimes(kfname, (uintptr_t) (tv + 1));
	printf("%s(%s, %p) = %s\n", TEST_SYSCALL_STR,
	       qname, tv + 1, errstr);

	k_utimes(kfname, (uintptr_t) (tv + 2));
	printf("%s(%s, %p) = %s\n", TEST_SYSCALL_STR,
	       qname, tv + 2, errstr);

	tv[0].tv_sec = 0xdeadbeefU;
	tv[0].tv_usec = 0xfacefeedU;
	tv[1].tv_sec = (typeof(tv[1].tv_sec)) 0xcafef00ddeadbeefLL;
	tv[1].tv_usec = (suseconds_t) 0xbadc0dedfacefeedLL;

	k_utimes(kfname, (uintptr_t) tv);
	printf("%s(%s, [", TEST_SYSCALL_STR, qname);
	print_tv(&tv[0]);
	printf(", ");
	print_tv(&tv[1]);
	printf("]) = %s\n", errstr);

	tv[0].tv_sec = 1492358607;
	tv[0].tv_usec = 1000000;
	tv[1].tv_sec = 1492356078;
	tv[1].tv_usec = 1000001;

	k_utimes(kfname, (uintptr_t) tv);
	printf("%s(%s, [", TEST_SYSCALL_STR, qname);
	print_tv(&tv[0]);
	printf(", ");
	print_tv(&tv[1]);
	printf("]) = %s\n", errstr);

	tv[0].tv_usec = 345678;
	tv[1].tv_usec = 456789;

	k_utimes(kfname, (uintptr_t) tv);
	printf("%s(%s, [", TEST_SYSCALL_STR, qname);
	print_tv(&tv[0]);
	printf(", ");
	print_tv(&tv[1]);
	printf("]) = %s\n", errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		k_utimes(kfname, f8ill_ptr_to_kulong(tv));
		printf("%s(%s, %#jx) = %s\n", TEST_SYSCALL_STR,
		       qname, (uintmax_t) f8ill_ptr_to_kulong(tv), errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
