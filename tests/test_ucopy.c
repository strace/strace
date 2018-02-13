/*
 * Test whether process_vm_readv and PTRACE_PEEKDATA work.
 *
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2018 The strace developers.
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

#include <errno.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/wait.h>

#include "test_ucopy.h"

#ifndef HAVE_PROCESS_VM_READV

# include <asm/unistd.h>
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
