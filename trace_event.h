/*
 * Copyright (c) 2017 The strace developers.
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

#ifndef STRACE_TRACE_EVENT_H
#define STRACE_TRACE_EVENT_H

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
};

#endif /* !STRACE_TRACE_EVENT_H */
