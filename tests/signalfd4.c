/*
 * Check decoding of signalfd4 syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2017 The strace developers.
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
#include <fcntl.h>
#include <asm/unistd.h>

#if defined HAVE_SYS_SIGNALFD_H \
 && defined HAVE_SIGNALFD \
 && defined O_CLOEXEC

# include <signal.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/signalfd.h>

int
main(void)
{
	const char *const sigs = SIGUSR2 < SIGCHLD ? "USR2 CHLD" : "CHLD USR2";
	const unsigned int size = get_sigset_size();

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGCHLD);

	int fd = signalfd(-1, &mask, O_CLOEXEC | O_NONBLOCK);
	printf("signalfd4(-1, [%s], %u, SFD_CLOEXEC|SFD_NONBLOCK) = %s\n",
	       sigs, size, sprintrc(fd));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_SIGNALFD_H && HAVE_SIGNALFD && O_CLOEXEC")

#endif
