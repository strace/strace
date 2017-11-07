/*
 * Check handling of seccomp SECCOMP_RET_TRAP.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <asm/unistd.h>

#ifdef HAVE_PRCTL
# include <sys/prctl.h>
#endif
#ifdef HAVE_LINUX_SECCOMP_H
# include <linux/seccomp.h>
#endif
#include <linux/filter.h>

#if defined __NR_seccomp \
 && defined PR_SET_NO_NEW_PRIVS \
 && defined SECCOMP_SET_MODE_FILTER \
 && defined SECCOMP_RET_TRAP \
 && defined BPF_JUMP \
 && defined BPF_STMT

static struct sock_filter filter[] = {
	/* load syscall number */
	BPF_STMT(BPF_LD|BPF_W|BPF_ABS, offsetof(struct seccomp_data, nr)),

	/* trap nanosleep syscall */
	BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, __NR_nanosleep, 0, 1),
	BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_TRAP),

	/* continue with the system call */
	BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW)
};

static const struct sock_fprog prog = {
	.filter = filter, .len = ARRAY_SIZE(filter)
};

static sigset_t mask;

static const struct timespec ts = { 0, 0 };

static void
handler(int sig)
{
}

int
main(void)
{
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
		perror_msg_and_skip("PR_SET_NO_NEW_PRIVS");

	if (syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, 0, &prog))
		perror_msg_and_skip("SECCOMP_SET_MODE_FILTER");

	if (sigprocmask(SIG_SETMASK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	if (signal(SIGSYS, handler))
		perror_msg_and_fail("signal");

	if (chdir("."))
		perror_msg_and_fail("chdir");

	syscall(__NR_nanosleep, &ts, NULL);

	if (chdir("./"))
		perror_msg_and_fail("chdir");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_seccomp && PR_SET_NO_NEW_PRIVS"
		    " && SECCOMP_SET_MODE_FILTER && SECCOMP_RET_TRAP"
		    " && BPF_JUMP && BPF_STMT")

#endif
