/*
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TRACE_EVENT_H
# define STRACE_TRACE_EVENT_H

/* Possible trace event loop states. Returned by next_event() and dispatched by
 * dispatch_event(). */
enum trace_event {
	/* Break the main loop. */
	TE_BREAK,

	/* Call next_event() again. */
	TE_NEXT,

	/* Restart the tracee with signal 0 and call next_event() again. */
	TE_RESTART,

	/*
	 * For all the events below, current_tcp is set to current tracee's
	 * tcb.  All the suggested actions imply that you want to continue
	 * tracing of the current tracee; alternatively, you can detach it.
	 */

	/*
	 * Syscall entry or exit.
	 * Restart the tracee with signal 0, or with an injected signal number.
	 */
	TE_SYSCALL_STOP,

	/*
	 * Tracee received signal with number WSTOPSIG(*pstatus); signal info
	 * is written to *si.  Restart the tracee (with that signal number
	 * if you want to deliver it).
	 */
	TE_SIGNAL_DELIVERY_STOP,

	/*
	 * Tracee was killed by a signal with number WTERMSIG(*pstatus).
	 */
	TE_SIGNALLED,

	/*
	 * Tracee was stopped by a signal with number WSTOPSIG(*pstatus).
	 * Restart the tracee with that signal number.
	 */
	TE_GROUP_STOP,

	/*
	 * Tracee exited with status WEXITSTATUS(*pstatus).
	 */
	TE_EXITED,

	/*
	 * Tracee is going to perform execve().
	 * Restart the tracee with signal 0.
	 */
	TE_STOP_BEFORE_EXECVE,

	/*
	 * Tracee is going to terminate.
	 * Restart the tracee with signal 0.
	 */
	TE_STOP_BEFORE_EXIT,

	/*
	 * SECCOMP_RET_TRACE rule is triggered.
	 */
	TE_SECCOMP,
};

#endif /* !STRACE_TRACE_EVENT_H */
