/*
 * Check decoding of getcpu syscall.
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

#ifdef __NR_getcpu

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	unsigned *bogus_cpu =
		(unsigned *) tail_alloc(sizeof(* bogus_cpu)) + 1;
	unsigned *bogus_node =
		(unsigned *) tail_alloc(sizeof(* bogus_node)) + 1;
	unsigned *bogus_tcache =
		(unsigned *) tail_alloc(sizeof(* bogus_tcache)) + 1;

	long res;
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned, cpu);
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned, node);
	long * tcache = tail_alloc(128);

	res = syscall(__NR_getcpu, NULL, NULL, NULL);
	printf("getcpu(NULL, NULL, NULL) = %s\n", sprintrc(res));

	res = syscall(__NR_getcpu, bogus_cpu, bogus_node, bogus_tcache);
	printf("getcpu(%p, %p, %p) = %s\n",
	       bogus_cpu, bogus_node, bogus_tcache, sprintrc(res));

	res = syscall(__NR_getcpu, cpu, node, tcache);
	if (res != 0)
		perror_msg_and_skip("getcpu");

	printf("getcpu([%u], [%u], %p) = 0\n", *cpu, *node, tcache);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getcpu");

#endif
