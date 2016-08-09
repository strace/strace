/*
 * Check verbose decoding of prctl PR_SET_SECCOMP SECCOMP_MODE_FILTER.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <asm/unistd.h>

#ifdef HAVE_PRCTL
# include <sys/prctl.h>
#endif
#ifdef HAVE_LINUX_SECCOMP_H
# include <linux/seccomp.h>
#endif
#ifdef HAVE_LINUX_FILTER_H
# include <linux/filter.h>
#endif

#if defined HAVE_PRCTL \
 && defined PR_SET_NO_NEW_PRIVS \
 && defined PR_SET_SECCOMP \
 && defined SECCOMP_MODE_FILTER \
 && defined SECCOMP_RET_ERRNO \
 && defined BPF_JUMP \
 && defined BPF_STMT

#define SOCK_FILTER_ALLOW_SYSCALL(nr) \
		BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, __NR_ ## nr, 0, 1), \
		BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW)

#define SOCK_FILTER_DENY_SYSCALL(nr, err) \
		BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, __NR_ ## nr, 0, 1), \
		BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ERRNO|(SECCOMP_RET_DATA & (err)))

#define SOCK_FILTER_KILL_PROCESS \
		BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_KILL)

#define PRINT_ALLOW_SYSCALL(nr) \
	printf("BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, %#x, 0, 0x1), " \
	       "BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW), ", \
	       __NR_ ## nr)

#define PRINT_DENY_SYSCALL(nr, err) \
	printf("BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, %#x, 0, 0x1), " \
	       "BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ERRNO|%#x), ", \
	       __NR_ ## nr, err)

static const struct sock_filter filter[] = {
	/* load syscall number */
	BPF_STMT(BPF_LD|BPF_W|BPF_ABS, offsetof(struct seccomp_data, nr)),

	/* allow syscalls */
	SOCK_FILTER_ALLOW_SYSCALL(close),
	SOCK_FILTER_ALLOW_SYSCALL(exit),
	SOCK_FILTER_ALLOW_SYSCALL(exit_group),

	/* deny syscalls */
	SOCK_FILTER_DENY_SYSCALL(sync, EBUSY),
	SOCK_FILTER_DENY_SYSCALL(setsid, EPERM),

	/* kill process */
	SOCK_FILTER_KILL_PROCESS
};

static const struct sock_fprog prog = {
	.len = ARRAY_SIZE(filter),
	.filter = (struct sock_filter *) filter,
};

int
main(void)
{
	int fds[2];

	puts("prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)  = 0");

	printf("prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, {len=%u, filter=[",
	       prog.len);

	printf("BPF_STMT(BPF_LD|BPF_W|BPF_ABS, %#x), ",
	       (unsigned) offsetof(struct seccomp_data, nr));

	PRINT_ALLOW_SYSCALL(close);
	PRINT_ALLOW_SYSCALL(exit);
	PRINT_ALLOW_SYSCALL(exit_group);

	PRINT_DENY_SYSCALL(sync, EBUSY),
	PRINT_DENY_SYSCALL(setsid, EPERM),

	printf("BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_KILL)");

	puts("]}) = 0");
	puts("+++ exited with 0 +++");

	fflush(stdout);
	close(0);
	close(1);

	if (pipe(fds))
		perror_msg_and_fail("pipe");
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
		perror_msg_and_skip("PR_SET_NO_NEW_PRIVS");
	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog))
		perror_msg_and_skip("PR_SET_SECCOMP");
	if (close(0) || close(1))
		_exit(77);

	_exit(0);
}

#else

SKIP_MAIN_UNDEFINED("HAVE_PRCTL && PR_SET_NO_NEW_PRIVS && PR_SET_SECCOMP"
		    " && SECCOMP_MODE_FILTER && SECCOMP_RET_ERRNO"
		    " && BPF_JUMP && BPF_STMT")

#endif
