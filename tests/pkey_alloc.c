/*
 * Check decoding of pkey_alloc syscall.
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

#ifdef __NR_pkey_alloc

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const kernel_ulong_t flags[] = {
		0,
		(kernel_ulong_t) 0xbadc0ded00000000ULL,
		(kernel_ulong_t) 0xffff0000eeee1111ULL,
		(kernel_ulong_t) 0x123456789abcdef0ULL,
	};
	static const struct {
		kernel_ulong_t val;
		const char *str;
	} rights[] = {
		{ (kernel_ulong_t) 0xbadc0ded00000002ULL,
			sizeof(kernel_ulong_t) > sizeof(int) ?
			"PKEY_DISABLE_WRITE|0xbadc0ded00000000" :
			"PKEY_DISABLE_WRITE" },
		{ 0xdec0ded, "PKEY_DISABLE_ACCESS|0xdec0dec" },
		{ 0x3, "PKEY_DISABLE_ACCESS|PKEY_DISABLE_WRITE" },
		{ ARG_STR(0) },
		{ 0xbadc0dec, "0xbadc0dec /* PKEY_??? */" },
	};

	long rc;
	unsigned int i;
	unsigned int j;

	for (i = 0; i < ARRAY_SIZE(flags); i++) {
		for (j = 0; j < ARRAY_SIZE(rights); j++) {
			rc = syscall(__NR_pkey_alloc, flags[i], rights[j].val);
			printf("pkey_alloc(%#llx, %s) = %s\n",
			       (unsigned long long) flags[i], rights[j].str,
			       sprintrc(rc));
		}
	}

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pkey_alloc");

#endif
