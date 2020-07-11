/*
 * Check decoding of pidfd_send_signal syscall.
 *
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <unistd.h>
#include "scno.h"
#include "pidns.h"

#ifdef __NR_pidfd_send_signal

# include <fcntl.h>
# include <stdio.h>
# include <signal.h>

static const char *errstr;

static long
sys_pidfd_send_signal(int pidfd, int sig, const void *info, int flags)
{
	kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	kernel_ulong_t arg1 = fill | (unsigned int) pidfd;
	kernel_ulong_t arg2 = fill | (unsigned int) sig;
	kernel_ulong_t arg3 = (unsigned long) info;
	kernel_ulong_t arg4 = fill | (unsigned int) flags;

	long rc = syscall(__NR_pidfd_send_signal, arg1, arg2, arg3, arg4);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	PIDNS_TEST_INIT;

	static const char null_path[] = "/dev/null";

	int fd = open(null_path, O_RDONLY);
	if (fd < 0)
		perror_msg_and_fail("open: %s", null_path);

	TAIL_ALLOC_OBJECT_CONST_PTR(siginfo_t, si);
	const void *esi = (const void *) si + 1;

	sys_pidfd_send_signal(fd, SIGUSR1, esi, 0);
	pidns_print_leader();
	printf("pidfd_send_signal(%d, SIGUSR1, %p, 0) = %s\n",
	       fd, esi, errstr);

	si->si_signo = SIGUSR1;
	si->si_code = SI_QUEUE;
	si->si_pid = getpid();

	sys_pidfd_send_signal(fd, SIGUSR2, si, -1);
	pidns_print_leader();
	printf("pidfd_send_signal(%d, SIGUSR2, {si_signo=SIGUSR1"
	       ", si_code=SI_QUEUE, si_errno=%u, si_pid=%d%s, si_uid=%d"
	       ", si_value={int=%d, ptr=%p}}, %#x) = %s\n",
	       fd, si->si_errno, si->si_pid, pidns_pid2str(PT_TGID), si->si_uid,
	       si->si_int, si->si_ptr, -1U, errstr);

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pidfd_send_signal")

#endif
