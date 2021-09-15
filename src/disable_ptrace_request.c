/*
 * A helper that executes the specified program
 * with the ptrace request disabled.
 *
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ptrace.h"
#include "scno.h"
#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <linux/filter.h>
#include <linux/seccomp.h>

#ifndef HAVE_PROGRAM_INVOCATION_NAME
char *program_invocation_name;
#endif

void ATTRIBUTE_NORETURN
die(void)
{
	exit(1);
}

static void
init(int argc, char **argv)
{
	if (!program_invocation_name || !*program_invocation_name) {
		static char name[] = DEFAULT_PROGRAM_INVOCATION_NAME;
		program_invocation_name =
			(argc > 0 && argv[0] && *argv[0]) ? argv[0] : name;
	}
}

#if defined DISABLE_PTRACE_REQUEST \
 && defined PR_SET_NO_NEW_PRIVS \
 && defined PR_SET_SECCOMP \
 && defined BPF_JUMP \
 && defined BPF_STMT \
 && defined HAVE_FORK

static unsigned int
get_arch(void)
{
	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_die("fork");

	if (pid == 0) {
		/* get the pid before PTRACE_TRACEME */
		pid = getpid();
		if (ptrace(PTRACE_TRACEME, 0, 0, 0))
			perror_msg_and_die("PTRACE_TRACEME");
		kill(pid, SIGSTOP);
		/* unreachable */
		_exit(1);
	}

	int status = 0;
	if (waitpid(pid, &status, 0) != pid ||
	    !WIFSTOPPED(status) ||
	    WSTOPSIG(status) != SIGSTOP) {
		/* cannot happen */
		perror_msg_and_die("waitpid: status = %d", status);
	}

	static const unsigned int size =
		offsetof(struct_ptrace_syscall_info, entry);
	struct_ptrace_syscall_info psi = { .arch = 0 };

	long rc = ptrace(PTRACE_GET_SYSCALL_INFO, pid, size, &psi);

	int saved_errno = errno;
	(void) kill(pid, SIGKILL);
	(void) waitpid(pid, NULL, 0);
	errno = saved_errno;

	/*
	 * Skip if PTRACE_GET_SYSCALL_INFO is not available
	 * or behaves in an unexpected way.
	 */
	if (rc < (long) size ||
	    psi.op != PTRACE_SYSCALL_INFO_NONE ||
	    psi.arch == 0) {
		perror_msg_and_die("PTRACE_GET_SYSCALL_INFO");
	}

	return psi.arch;
}

int
main(int argc, char **argv)
{
	init(argc, argv);

	if (argc < 2)
		error_msg_and_die("Insufficient arguments");

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
		perror_msg_and_die("PR_SET_NO_NEW_PRIVS");

	struct sock_filter filter[] = {
		/* load the architecture */
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
			 offsetof(struct seccomp_data, arch)),
		/* jump to "allow" if the architecture does not match */
		BPF_JUMP(BPF_JMP | BPF_K | BPF_JEQ, get_arch(), 0, 5),
		/* load the syscall number */
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
			 offsetof(struct seccomp_data, nr)),
		/* jump to "allow" if it is not equal to __NR_ptrace */
		BPF_JUMP(BPF_JMP | BPF_K | BPF_JEQ, __NR_ptrace, 0, 3),
		/* load the 1st syscall argument */
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
			 offsetof(struct seccomp_data, args[0])
			 + (is_bigendian ? sizeof(uint32_t) : 0)),
		/* jump to "allow" if it is not equal to DISABLE_PTRACE_REQUEST */
		BPF_JUMP(BPF_JMP | BPF_K | BPF_JEQ, DISABLE_PTRACE_REQUEST, 0, 1),
		/* reject */
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | EIO),
		/* allow */
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)
	};

	const struct sock_fprog prog = {
		.len = ARRAY_SIZE(filter),
		.filter = filter,
	};

	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog))
		perror_msg_and_die("PR_SET_SECCOMP");

	(void) execvp(argv[1], argv + 1);
	perror_msg_and_die("execvp: %s", argv[1]);
}

#else

int
main(int argc, char **argv)
{
	init(argc, argv);
	error_msg_and_die("Operation not supported");
}

#endif
