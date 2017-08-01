/*
 * Force legacy printpath/umovestr using PR_SET_DUMPABLE.
 *
 * Copyright (c) 2017 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_PRCTL
# include <sys/prctl.h>
#endif

#if defined HAVE_PRCTL && defined PR_SET_DUMPABLE

# include <stdio.h>
# include <unistd.h>

# include "test_ucopy.h"

int
main(void)
{
	if (!test_process_vm_readv())
		perror_msg_and_skip("process_vm_readv");

	/*
	 * Clearing dumpable flag disallows process_vm_readv.
	 * If the kernel does not contain commit
	 * 84d77d3f06e7e8dea057d10e8ec77ad71f721be3, then
	 * PTRACE_PEEKDATA remains allowed.
	 */
	if (prctl(PR_SET_DUMPABLE, 0))
		perror_msg_and_skip("PR_SET_DUMPABLE 0");

	if (!test_ptrace_peekdata())
		perror_msg_and_skip("PTRACE_PEEKDATA");

	test_printpath(sizeof(long) * 4);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_PRCTL && PR_SET_DUMPABLE")

#endif
