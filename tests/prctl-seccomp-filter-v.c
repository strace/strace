/*
 * Check verbose decoding of prctl PR_SET_SECCOMP SECCOMP_MODE_FILTER.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/prctl.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include "scno.h"

#if defined BPF_JUMP && defined BPF_STMT

# define SOCK_FILTER_ALLOW_SYSCALL(nr) \
		BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, __NR_ ## nr, 0, 1), \
		BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW)

# define SOCK_FILTER_DENY_SYSCALL(nr, err) \
		BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, __NR_ ## nr, 0, 1), \
		BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ERRNO|(SECCOMP_RET_DATA & (err)))

# define SOCK_FILTER_KILL_PROCESS \
		BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_KILL)

# define PRINT_ALLOW_SYSCALL(nr) \
	printf("BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, %#lx, 0, 0x1), " \
	       "BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW), ", \
	       (long) __NR_ ## nr)

# define PRINT_DENY_SYSCALL(nr, err) \
	printf("BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, %#lx, 0, 0x1), " \
	       "BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ERRNO|%#x), ", \
	       (long) __NR_ ## nr, err)

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

	prctl_marker();

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

	printf("BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_KILL_THREAD)");

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

SKIP_MAIN_UNDEFINED("BPF_JUMP && BPF_STMT")

#endif
