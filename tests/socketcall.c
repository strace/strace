/*
 * Check decoding of socketcall syscall.
 *
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

#ifdef __NR_socketcall

# include <assert.h>
# include <stdio.h>
# include <unistd.h>

# include "xlat.h"
# include "xlat/socketcalls.h"

static const char *
xlookup_uint(const struct xlat *xlat, const unsigned int val)
{
	for (; xlat->str != NULL; xlat++)
		if (xlat->val == val)
			return xlat->str;
	return NULL;
}

static const int sc_min = 1, sc_max = 20;
static void *efault;

static void
test_socketcall(const int i, const void *const addr)
{
	const unsigned long call =
		(unsigned long) 0xfacefeed00000000ULL | (unsigned int) i;

	long rc = syscall(__NR_socketcall, call, addr);

	if (i < sc_min || i > sc_max) {
		printf("socketcall(%d, %p) = %ld %s (%m)\n",
		       (int) call, addr, rc, errno2name());
	} else if (addr == efault) {
		const char *const str = xlookup_uint(socketcalls, i);
		assert(str);
		printf("socketcall(%s, %p) = %ld %s (%m)\n",
		       str, addr, rc, errno2name());
	}
}
int
main(void)
{
	assert((unsigned) sc_min == socketcalls[0].val);
	assert((unsigned) sc_max == socketcalls[ARRAY_SIZE(socketcalls) - 2].val);

	const unsigned long *const args = tail_alloc(sizeof(*args) * 6);
	efault = tail_alloc(1) + 1;

	int i;
	for (i = sc_min - 3; i <= sc_max + 3; ++i) {
		test_socketcall(i, efault);
		test_socketcall(i, args);
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_socketcall")

#endif
