/*
 * Check verbose decoding of seccomp SECCOMP_SET_MODE_FILTER.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include "scno.h"

#if defined PR_SET_NO_NEW_PRIVS \
 && defined BPF_JUMP \
 && defined BPF_STMT

# define SOCK_FILTER_ALLOW_SYSCALL(nr) \
		BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, __NR_ ## nr, 0, 1), \
		BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW)

# define SOCK_FILTER_DENY_SYSCALL(nr, err) \
		BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, __NR_ ## nr, 0, 1), \
		BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ERRNO|(SECCOMP_RET_DATA & (err)))

# define SOCK_FILTER_KILL_PROCESS \
		BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_KILL)

# define PRINT_ALLOW_SYSCALL(nr) \
	tprintf("BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, %#lx, 0, 0x1), " \
	        "BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW), ", \
	        (long) __NR_ ## nr)

# define PRINT_DENY_SYSCALL(nr, err) \
	tprintf("BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, %#lx, 0, 0x1), " \
	        "BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ERRNO|%#x), ", \
	        (long) __NR_ ## nr, err)

static const struct sock_filter filter_c[] = {
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

int
main(void)
{
	tprintf("%s", "");

	static const char kill_stmt_txt[] =
		"BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_KILL_THREAD)";
	struct sock_filter *const filter =
		tail_memdup(filter_c, sizeof(filter_c));
	struct sock_filter *const big_filter =
		tail_alloc(sizeof(*big_filter) * (BPF_MAXINSNS + 1));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct sock_fprog, prog);

	int fds[2];
	if (pipe(fds))
		perror_msg_and_fail("pipe");
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
		perror_msg_and_skip("PR_SET_NO_NEW_PRIVS");

	prog->filter = filter +  ARRAY_SIZE(filter_c);
	prog->len = 1;
	syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, 0, prog);
	tprintf("seccomp(SECCOMP_SET_MODE_FILTER, 0, {len=1, filter=%p})"
		" = -1 EFAULT (%m)\n", prog->filter);

	prog->filter = filter +  ARRAY_SIZE(filter_c) - 1;
	prog->len = 3;
	syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, 0, prog);
	tprintf("seccomp(SECCOMP_SET_MODE_FILTER, 0, {len=%u"
		", filter=[%s, ... /* %p */]}) = -1 EFAULT (%m)\n",
		prog->len, kill_stmt_txt, filter +  ARRAY_SIZE(filter_c));

	prog->len = 0;
	syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, 0, prog);
	tprintf("seccomp(SECCOMP_SET_MODE_FILTER, 0, {len=0, filter=[]})"
		" = -1 EINVAL (%m)\n");

	for (unsigned int i = 0; i <= BPF_MAXINSNS; ++i) {
		const struct sock_filter stmt =
			BPF_STMT(BPF_CLASS(i), i << 16);
		big_filter[i] = stmt;
	}

	prog->filter = big_filter;
	prog->len = BPF_MAXINSNS + 1;
	tprintf("seccomp(SECCOMP_SET_MODE_FILTER, %s, {len=%u, filter=[",
		"SECCOMP_FILTER_FLAG_TSYNC|SECCOMP_FILTER_FLAG_LOG|"
		"SECCOMP_FILTER_FLAG_SPEC_ALLOW|"
		"SECCOMP_FILTER_FLAG_NEW_LISTENER|"
		"SECCOMP_FILTER_FLAG_TSYNC_ESRCH|"
		"SECCOMP_FILTER_FLAG_WAIT_KILLABLE_RECV|"
		"0xffffffc0",
		prog->len);
	for (unsigned int i = 0; i < BPF_MAXINSNS; ++i) {
		if (i)
			tprintf(", ");
		switch (BPF_CLASS(i)) {
		case BPF_LD:
			tprintf("BPF_STMT(BPF_LD|BPF_W|BPF_IMM, %#x)", i << 16);
			break;
		case BPF_LDX:
			tprintf("BPF_STMT(BPF_LDX|BPF_W|BPF_IMM, %#x)", i << 16);
			break;
		case BPF_ST:
			tprintf("BPF_STMT(BPF_ST, %#x)", i << 16);
			break;
		case BPF_STX:
			tprintf("BPF_STMT(BPF_STX, %#x)", i << 16);
			break;
		case BPF_ALU:
			tprintf("BPF_STMT(BPF_ALU|BPF_K|BPF_ADD, %#x)", i << 16);
			break;
		case BPF_JMP:
			tprintf("BPF_STMT(BPF_JMP|BPF_K|BPF_JA, %#x)", i << 16);
			break;
		case BPF_RET:
			tprintf("BPF_STMT(BPF_RET|BPF_K, %#x"
				" /* SECCOMP_RET_??? */)", i << 16);
			break;
		case BPF_MISC:
			tprintf("BPF_STMT(BPF_MISC|BPF_TAX, %#x)", i << 16);
			break;
		}
	}
	tprintf(", ...]})");
	syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, -1, prog);
	tprintf(" = -1 EINVAL (%m)\n");

	prog->filter = filter;
	prog->len = ARRAY_SIZE(filter_c);

	tprintf("seccomp(SECCOMP_SET_MODE_FILTER, 0, {len=%u, filter=[",
		prog->len);

	tprintf("BPF_STMT(BPF_LD|BPF_W|BPF_ABS, %#x), ",
	       (unsigned) offsetof(struct seccomp_data, nr));

	PRINT_ALLOW_SYSCALL(close);
	PRINT_ALLOW_SYSCALL(exit);
	PRINT_ALLOW_SYSCALL(exit_group);

	PRINT_DENY_SYSCALL(sync, EBUSY),
	PRINT_DENY_SYSCALL(setsid, EPERM),

	tprintf("%s]}) = 0\n+++ exited with 0 +++\n", kill_stmt_txt);

	if (syscall(__NR_seccomp, SECCOMP_SET_MODE_FILTER, 0, prog))
		perror_msg_and_skip("SECCOMP_SET_MODE_FILTER");

	if (close(0) || close(1))
		_exit(77);

	_exit(0);
}

#else

SKIP_MAIN_UNDEFINED("PR_SET_NO_NEW_PRIVS && BPF_JUMP && BPF_STMT")

#endif
