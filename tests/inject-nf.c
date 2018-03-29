/*
 * Check decoding of return values injected into a syscall that "never fails".
 *
 * Copyright (c) 2018 The strace developers.
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/unistd.h>

#include "raw_syscall.h"

#ifdef __alpha__
/* alpha has no getpid */
# define SC_NR __NR_getpgrp
# define SC_NAME "getpgrp"
# define getpid getpgrp
#else
# define SC_NR __NR_getpid
# define SC_NAME "getpid"
#endif

#ifdef raw_syscall_0
# define INVOKE_SC(err) raw_syscall_0(SC_NR, &err)
#else
/* No raw_syscall_0, let's use getpid() and hope for the best.  */
# define INVOKE_SC(err) getpid()
#endif

/*
 * This prototype is intentionally different
 * from the prototype provided by <unistd.h>.
 */
extern kernel_ulong_t getpid(void);

int
main(int ac, char **av)
{
	assert(ac == 1 || ac == 2);

	kernel_ulong_t expected =
		(ac == 1) ? getpid() : strtoull(av[1], NULL, 0);
	kernel_ulong_t err = 0;
	kernel_ulong_t rc = INVOKE_SC(err);

	if (err || rc != expected)
		error_msg_and_fail("expected %#llx, got rval=%#llx err=%#llx",
				   (unsigned long long) expected,
				   (unsigned long long) rc,
				   (unsigned long long) err);

	if (ac == 2) {
		printf("%s() = %llu (INJECTED)\n",
		       SC_NAME, (unsigned long long) rc);

		puts("+++ exited with 0 +++");
	}

	return 0;
}
