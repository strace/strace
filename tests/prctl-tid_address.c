/*
 * Check decoding of prctl PR_GET_TID_ADDRESS operation.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
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
#include <linux/prctl.h>

#if defined __NR_prctl && defined __NR_set_tid_address && \
	defined PR_GET_TID_ADDRESS

# include <inttypes.h>
# include <stdio.h>
# include <unistd.h>

static const char *
sprintaddr(kernel_ulong_t addr)
{
	static char buf[sizeof("0x") + sizeof(addr) * 2];

	if (addr) {
		snprintf(buf, sizeof(buf), "%#llx", (unsigned long long) addr);

		return buf;
	}

	return "NULL";
}

int
main(void)
{
	static const kernel_ulong_t bogus_addr =
		(kernel_ulong_t) 0xfffffffffffffffdULL;

	/* Note that kernel puts kernel-sized pointer even on x32 */
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_ulong_t, ptr);
	long rc;
	long set_ok;

	*ptr = (kernel_ulong_t) 0xbadc0dedda7a1057ULL;

	rc = syscall(__NR_prctl, PR_GET_TID_ADDRESS, NULL);
	printf("prctl(PR_GET_TID_ADDRESS, NULL) = %s\n", sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_TID_ADDRESS, bogus_addr);
	printf("prctl(PR_GET_TID_ADDRESS, %#llx) = %s\n",
	       (unsigned long long) bogus_addr, sprintrc(rc));

	rc = syscall(__NR_prctl, PR_GET_TID_ADDRESS, ptr);
	if (rc) {
		printf("prctl(PR_GET_TID_ADDRESS, %p) = %s\n",
		       ptr, sprintrc(rc));
	} else {
		printf("prctl(PR_GET_TID_ADDRESS, [%s]) = %s\n",
		       sprintaddr(*ptr), sprintrc(rc));
	}

	set_ok = syscall(__NR_set_tid_address, bogus_addr);

	rc = syscall(__NR_prctl, PR_GET_TID_ADDRESS, ptr);
	if (rc) {
		printf("prctl(PR_GET_TID_ADDRESS, %p) = %s\n",
		       ptr, sprintrc(rc));
	} else {
		printf("prctl(PR_GET_TID_ADDRESS, [%s]) = %s\n",
		       sprintaddr(set_ok ? bogus_addr : *ptr), sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl && __NR_set_tid_address && PR_GET_TID_ADDRESS")

#endif
