/*
 * Check decoding of prctl operations which use arg2 as pointer to an integer
 * value: PR_GET_CHILD_SUBREAPER, PR_GET_ENDIAN, PR_GET_FPEMU, and PR_GET_FPEXC.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
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

#include "tests.h"
#include <asm/unistd.h>

#if defined __NR_prctl

# include <stdint.h>
# include <stdio.h>
# include <unistd.h>
# include <linux/prctl.h>

static const char *errstr;

static long
prctl(kernel_ulong_t arg1, kernel_ulong_t arg2)
{
	static const kernel_ulong_t bogus_arg =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;
	long rc = syscall(__NR_prctl, arg1, arg2, bogus_arg);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const kernel_ulong_t bogus_addr1 =
		(kernel_ulong_t) 0x1e55c0de00000000ULL;
	static const kernel_ulong_t bogus_addr2 =
		(kernel_ulong_t) 0xfffffffffffffffdULL;
	static const kernel_ulong_t bogus_op_bits =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	static const struct {
		kernel_ulong_t val;
		const char *str;
	} options[] = {
		{ 37, "PR_GET_CHILD_SUBREAPER" },
		{ 19, "PR_GET_ENDIAN" },
		{  9, "PR_GET_FPEMU" },
		{ 11, "PR_GET_FPEXC" },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, ptr);
	long rc;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(options); ++i) {
		prctl(options[i].val | bogus_op_bits, 0);
		printf("prctl(%s, NULL) = %s\n", options[i].str, errstr);

		if (bogus_addr1) {
			prctl(options[i].val | bogus_op_bits, bogus_addr1);
			printf("prctl(%s, %#llx) = %s\n", options[i].str,
			       (unsigned long long) bogus_addr1, errstr);
		}

		prctl(options[i].val | bogus_op_bits, bogus_addr2);
		printf("prctl(%s, %#llx) = %s\n", options[i].str,
		       (unsigned long long) bogus_addr2, errstr);

		prctl(options[i].val | bogus_op_bits, (uintptr_t) (ptr + 1));
		printf("prctl(%s, %p) = %s\n", options[i].str,
		       ptr + 1, errstr);

		rc = prctl(options[i].val | bogus_op_bits, (uintptr_t) ptr);
		if (!rc) {
			printf("prctl(%s, [%u]) = %s\n",
			       options[i].str, *ptr, errstr);
		} else {
			printf("prctl(%s, %p) = %s\n",
			       options[i].str, ptr, errstr);
		}

		if (F8ILL_KULONG_SUPPORTED) {
			kernel_ulong_t bogus_addr3 = f8ill_ptr_to_kulong(ptr);
			prctl(options[i].val | bogus_op_bits, bogus_addr3);
			printf("prctl(%s, %#llx) = %s\n", options[i].str,
			       (unsigned long long) bogus_addr3, errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl")

#endif
