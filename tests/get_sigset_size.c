/*
 * Find out the size of kernel's sigset_t.
 *
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@altlinux.org>
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
#include <signal.h>
#include <unistd.h>
#include <asm/unistd.h>

/*
 * If the sigset size specified to rt_sigprocmask is not equal to the size
 * of kernel's sigset_t, the kernel does not look at anything else and fails
 * with EINVAL.
 *
 * Otherwise, if both pointers specified to rt_sigprocmask are NULL,
 * the kernel just returns 0.
 *
 * This vaguely documented kernel feature can be used to probe
 * the kernel and find out the size of kernel's sigset_t.
 */

unsigned int
get_sigset_size(void)
{
	static unsigned int set_size;

	if (!set_size) {
		static const unsigned int big_size = 1024 / 8;

		for (set_size = big_size; set_size; set_size >>= 1) {
			if (!syscall(__NR_rt_sigprocmask, SIG_SETMASK,
				     NULL, NULL, set_size))
				break;
		}

		if (!set_size)
			perror_msg_and_fail("rt_sigprocmask");
	}

	return set_size;
}
