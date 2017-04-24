/*
 * Check decoding of futimesat syscall.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_futimesat

# include <stdint.h>
# include <stdio.h>
# include <sys/time.h>
# include <unistd.h>

static void
print_tv(const struct timeval *tv)
{
	printf("{tv_sec=%lld, tv_usec=%llu}",
	       (long long) tv->tv_sec,
	       zero_extend_signed_to_ull(tv->tv_usec));
	print_time_t_usec(tv->tv_sec,
			  zero_extend_signed_to_ull(tv->tv_usec), 1);
}

static const char *errstr;

static long
k_futimesat(const kernel_ulong_t dirfd,
	    const kernel_ulong_t pathname,
	    const kernel_ulong_t times)
{
	long rc = syscall(__NR_futimesat, dirfd, pathname, times);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const kernel_ulong_t bogus_fd =
		(kernel_ulong_t) 0xbadfaceddeadbeaf;
	static const kernel_ulong_t kfdcwd =
		(kernel_ulong_t) 0xdefaced00000000 | -100U;
	static const char proto_fname[] = "futimesat_sample";
	static const char qname[] = "\"futimesat_sample\"";

	char *const fname = tail_memdup(proto_fname, sizeof(proto_fname));
	const kernel_ulong_t kfname = (uintptr_t) fname;
	struct timeval *const tv = tail_alloc(sizeof(*tv) * 2);

	(void) close(0);

	/* dirfd */
	k_futimesat(0, kfname, 0);
	printf("futimesat(0, %s, NULL) = %s\n", qname, errstr);

	k_futimesat(bogus_fd, kfname, 0);
	printf("futimesat(%d, %s, NULL) = %s\n", (int) bogus_fd, qname, errstr);

	k_futimesat(-100U, kfname, 0);
	printf("futimesat(AT_FDCWD, %s, NULL) = %s\n", qname, errstr);

	k_futimesat(kfdcwd, kfname, 0);
	printf("futimesat(AT_FDCWD, %s, NULL) = %s\n", qname, errstr);

	/* pathname */
	k_futimesat(kfdcwd, 0, 0);
	printf("futimesat(AT_FDCWD, NULL, NULL) = %s\n", errstr);

	k_futimesat(kfdcwd, kfname + sizeof(proto_fname) - 1, 0);
	printf("futimesat(AT_FDCWD, \"\", NULL) = %s\n", errstr);

	fname[sizeof(proto_fname) - 1] = '+';
	k_futimesat(kfdcwd, kfname, 0);
	fname[sizeof(proto_fname) - 1] = '\0';
	printf("futimesat(AT_FDCWD, %p, NULL) = %s\n", fname, errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		k_futimesat(kfdcwd, f8ill_ptr_to_kulong(fname), 0);
		printf("futimesat(AT_FDCWD, %#jx, NULL) = %s\n",
		       (uintmax_t) f8ill_ptr_to_kulong(fname), errstr);
	}

	/* times */
	k_futimesat(kfdcwd, kfname, (uintptr_t) (tv + 1));
	printf("futimesat(AT_FDCWD, %s, %p) = %s\n",
	       qname, tv + 1, errstr);

	k_futimesat(kfdcwd, kfname, (uintptr_t) (tv + 2));
	printf("futimesat(AT_FDCWD, %s, %p) = %s\n",
	       qname, tv + 2, errstr);

	tv[0].tv_sec = 0xdeadbeefU;
	tv[0].tv_usec = 0xfacefeedU;
	tv[1].tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	tv[1].tv_usec = (long) 0xbadc0dedfacefeedLL;

	k_futimesat(kfdcwd, kfname, (uintptr_t) tv);
	printf("futimesat(AT_FDCWD, %s, [", qname);
	print_tv(&tv[0]);
	printf(", ");
	print_tv(&tv[1]);
	printf("]) = %s\n", errstr);

	tv[0].tv_sec = 1492356708;
	tv[0].tv_usec = 567891234;
	tv[1].tv_sec = 1492357086;
	tv[1].tv_usec = 678902345;

	k_futimesat(kfdcwd, kfname, (uintptr_t) tv);
	printf("futimesat(AT_FDCWD, %s, [", qname);
	print_tv(&tv[0]);
	printf(", ");
	print_tv(&tv[1]);
	printf("]) = %s\n", errstr);

	tv[0].tv_usec = 567891;
	tv[1].tv_usec = 678902;

	k_futimesat(kfdcwd, kfname, (uintptr_t) tv);
	printf("futimesat(AT_FDCWD, %s, [", qname);
	print_tv(&tv[0]);
	printf(", ");
	print_tv(&tv[1]);
	printf("]) = %s\n", errstr);

	if (F8ILL_KULONG_SUPPORTED) {
		k_futimesat(kfdcwd, kfname, f8ill_ptr_to_kulong(tv));
		printf("futimesat(AT_FDCWD, %s, %#jx) = %s\n",
		       qname, (uintmax_t) f8ill_ptr_to_kulong(tv), errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_futimesat")

#endif
