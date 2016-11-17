/*
 * Copyright (c) 2016 JingPiao Chen <chenjingpiao@foxmail.com>
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

#if defined __NR_prctl && defined PR_GET_PDEATHSIG && defined PR_SET_PDEATHSIG

# include <stdio.h>
# include <unistd.h>
# include <sys/signal.h>

int
main(void)
{
	int pdeathsig;
	long rc;

	if ((rc = syscall(__NR_prctl, PR_SET_PDEATHSIG, SIGINT)))
		perror_msg_and_skip("prctl(PR_SET_PDEATHSIG)");
	printf("prctl(PR_SET_PDEATHSIG, SIGINT) = %s\n", sprintrc(rc));

	if ((rc = syscall(__NR_prctl, PR_GET_PDEATHSIG, &pdeathsig)))
		perror_msg_and_skip("prctl(PR_GET_PDEATHSIG)");
	printf("prctl(PR_GET_PDEATHSIG, [SIGINT]) = %s\n", sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl && PR_GET_PDEATHSIG && PR_SET_PDEATHSIG")

#endif
