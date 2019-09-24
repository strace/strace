/*
 * Test whether process_vm_readv and PTRACE_PEEKDATA work.
 *
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/wait.h>

#include "test_ucopy.h"

#ifndef HAVE_PROCESS_VM_READV

# include "scno.h"
static ssize_t
strace_process_vm_readv(pid_t pid,
		 const struct iovec *lvec,
		 unsigned long liovcnt,
		 const struct iovec *rvec,
		 unsigned long riovcnt,
		 unsigned long flags)
{
	return syscall(__NR_process_vm_readv,
		       (long) pid, lvec, liovcnt, rvec, riovcnt, flags);
}
# define process_vm_readv strace_process_vm_readv

#endif /* !HAVE_PROCESS_VM_READV */

static bool
call_process_vm_readv(const int pid, long *const addr)
{
	long data = 0;

	const struct iovec local = {
		.iov_base = &data,
		.iov_len = sizeof(data)
	};
	const struct iovec remote = {
		.iov_base = addr,
		.iov_len = sizeof(*addr)
	};

	return process_vm_readv(pid, &local, 1, &remote, 1, 0) == sizeof(data)
		&& data == 1;
}

static bool
call_ptrace_peekdata(const int pid, long *const addr)
{
	return ptrace(PTRACE_PEEKDATA, pid, addr, 0) == 1;
}

static bool
test_ucopy(bool (*fn)(int pid, long *addr))
{
	static long data;

	data = 0;
	bool rc = false;
	int saved = 0;

	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		data = 1;
		if (ptrace(PTRACE_TRACEME, 0, 0, 0))
			perror_msg_and_fail("PTRACE_TRACEME");
		raise(SIGSTOP);
		_exit(0);
	}

	for (;;) {
		int status, tracee;

		errno = 0;
		tracee = wait(&status);
		if (tracee != pid) {
			if (errno == EINTR)
				continue;
			saved = errno;
			kill(pid, SIGKILL);
			errno = saved;
			perror_msg_and_fail("wait");
		}
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0)
				break;
			error_msg_and_fail("unexpected exit status %u",
					   WEXITSTATUS(status));
		}
		if (WIFSIGNALED(status))
			error_msg_and_fail("unexpected signal %u",
					   WTERMSIG(status));
		if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP) {
			kill(pid, SIGKILL);
			error_msg_and_fail("unexpected wait status %x",
					   status);
		}

		errno = 0;
		rc = fn(pid, &data);
		if (!rc)
			saved = errno;

		if (ptrace(PTRACE_CONT, pid, 0, 0)) {
			saved = errno;
			kill(pid, SIGKILL);
			errno = saved;
			perror_msg_and_fail("PTRACE_CONT");
		}
	}

	if (!rc)
		errno = saved;
	return rc;
}

bool
test_process_vm_readv(void)
{
	return test_ucopy(call_process_vm_readv);
}

bool
test_ptrace_peekdata(void)
{
	return test_ucopy(call_ptrace_peekdata);
}
