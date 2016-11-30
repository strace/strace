/*
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
#include <limits.h>
#include <unistd.h>
#include <sys/resource.h>

static void
move_fd(int *from, int *to)
{
	for (; *to > *from; --*to) {
		if (dup2(*from, *to) != *to)
			continue;
		close(*from);
		*from = *to;
		break;
	}
}

void
pipe_maxfd(int pipefd[2])
{
	struct rlimit rlim;
	if (getrlimit(RLIMIT_NOFILE, &rlim))
		perror_msg_and_fail("getrlimit");
	if (rlim.rlim_cur < rlim.rlim_max) {
		struct rlimit rlim_new;
		rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
		if (!setrlimit(RLIMIT_NOFILE, &rlim_new))
			rlim.rlim_cur = rlim.rlim_max;
	}

	if (pipe(pipefd))
		perror_msg_and_fail("pipe");

	int max_fd = (rlim.rlim_cur > 0 && rlim.rlim_cur < INT_MAX)
		     ? rlim.rlim_cur - 1 : INT_MAX;

	move_fd(&pipefd[1], &max_fd);
	--max_fd;
	move_fd(&pipefd[0], &max_fd);
}
