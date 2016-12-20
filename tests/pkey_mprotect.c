/*
 * Check decoding of pkey_mprotect syscall.
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
#include "scno.h"

#ifdef __NR_pkey_mprotect

# include <stdio.h>
# include <unistd.h>
# include <sys/mman.h>

const char *
sprintptr(kernel_ulong_t ptr)
{
	static char buf[sizeof(ptr) * 2 + sizeof("0x")];

	if (ptr)
		snprintf(buf, sizeof(buf), "%#llx", (unsigned long long) ptr);
	else
		return "NULL";

	return buf;
}

int
main(void)
{
	static const kernel_ulong_t ptrs[] = {
		0,
		(kernel_ulong_t) 0xfacebeef00000000ULL,
		(kernel_ulong_t) 0xbadc0dedda7a1057ULL,
	};
	static const kernel_ulong_t sizes[] = {
		0,
		(kernel_ulong_t) 0xfacebeef00000000ULL,
		(kernel_ulong_t) 0xfedcba9876543210ULL,
		(kernel_ulong_t) 0x123456789abcdef0ULL,
		(kernel_ulong_t) 0xbadc0dedda7a1057ULL,
	};
	static const struct {
		kernel_ulong_t val;
		const char *str;
	} prots[] = {
		{ ARG_STR(PROT_READ) },
		/* For now, only 0x0300001f are used */
		{ (kernel_ulong_t) 0xdeadfeed00ca7500ULL,
			sizeof(kernel_ulong_t) > sizeof(int) ?
			"0xdeadfeed00ca7500 /* PROT_??? */" :
			"0xca7500 /* PROT_??? */" },
		{ ARG_STR(PROT_READ|PROT_WRITE|0xface00) },
	};
	static const kernel_ulong_t pkeys[] = {
		0,
		-1LL,
		(kernel_ulong_t) 0xface1e55,
		(kernel_ulong_t) 0xbadc0ded00000001,
	};

	long rc;
	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned int l;

	for (i = 0; i < ARRAY_SIZE(ptrs); i++) {
		for (j = 0; j < ARRAY_SIZE(sizes); j++) {
			for (k = 0; k < ARRAY_SIZE(prots); k++) {
				for (l = 0; l < ARRAY_SIZE(pkeys); l++) {
					rc = syscall(__NR_pkey_mprotect,
						     ptrs[i], sizes[j],
						     prots[k].val, pkeys[l]);
					printf("pkey_mprotect(%s, %llu, %s, %d)"
					       " = %s\n",
					       sprintptr(ptrs[i]),
					       (unsigned long long) sizes[j],
					       prots[k].str, (int) pkeys[l],
					       sprintrc(rc));
				}
			}
		}
	}

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pkey_mprotect");

#endif
