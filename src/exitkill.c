/*
 * Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kill_save_errno.h"
#include "ptrace.h"
#include "exitkill.h"

#include <signal.h>
#include <sys/wait.h>

#include "xlat/ptrace_syscall_info_op.h"

bool
is_exitkill_supported(void)
{
	static int exitkill_is_supported = -1;
	/*
	 * NOMMU provides no forks necessary for the test,
	 * leave the default unchanged.
	 */
#ifdef HAVE_FORK
	if (exitkill_is_supported < 0) {
		int pid = fork();
		if (pid < 0)
			perror_func_msg_and_die("fork");

		if (pid == 0) {
			pid = getpid();
			if (ptrace(PTRACE_TRACEME, 0L, 0L, 0L) < 0) {
				perror_func_msg_and_die("PTRACE_TRACEME");
			}
			GCOV_DUMP;
			kill(pid, SIGSTOP);
			_exit(1);
		}

		int status;
		unsigned long options =
			PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL;
		exitkill_is_supported =
			waitpid(pid, &status, 0) == pid &&
			WIFSTOPPED(status) &&
			WSTOPSIG(status) == SIGSTOP &&
			ptrace(PTRACE_SETOPTIONS, pid, 0L, options) == 0;

		kill_save_errno(pid, SIGKILL);
		waitpid(pid, NULL, 0);

		debug_msg("PTRACE_O_EXITKILL %s",
			  exitkill_is_supported ? "works" : "does not work");
	}
#endif /* HAVE_FORK */

	return exitkill_is_supported != 0;
}
