/*
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TCB_WAIT_DATA_H
#define STRACE_TCB_WAIT_DATA_H

#include "defs.h"

#include <signal.h>

#include "tracing_backend.h"

/**
 * Tracing-backend-agnostic data. Embedded inside tracing-backend-specific
 * structure.
 */
struct tcb_wait_data {
	enum trace_event te; /**< Event passed to dispatch_event() */
	union {
		unsigned int sig;  /**< Signal tracee got. */
		unsigned int exit; /**< Tracee's exit code. */
	};
	bool core_dumped;    /**< Wether core was dumped on termination. */
	siginfo_t si;        /**< siginfo, returned by PTRACE_GETSIGINFO */
};

static inline size_t
trace_wait_data_size(struct tcb *tcp)
{
	return cur_tracing_backend->trace_wait_data_size
		?: sizeof(struct tcb_wait_data);
}

static inline struct tcb_wait_data *
init_trace_wait_data(void *p)
{
	if (cur_tracing_backend->init_trace_wait_data)
		return cur_tracing_backend->init_trace_wait_data(p);

	/* Default implementation: naked struct tcb_wait_data */
	struct tcb_wait_data *wd = p;

	memset(wd, 0, sizeof(*wd));

	return wd;
}

static inline struct tcb_wait_data *
copy_trace_wait_data(struct tcb_wait_data *wd)
{
	if (cur_tracing_backend->copy_trace_wait_data)
		return cur_tracing_backend->copy_trace_wait_data(wd);

	/* Default implementation: naked struct tcb_wait_data */
	struct tcb_wait_data *new_wd = xmalloc(sizeof(*new_wd));

	memcpy(new_wd, wd, sizeof(*wd));

	return new_wd;
}

static inline void
free_trace_wait_data(struct tcb_wait_data *wd)
{
	if (cur_tracing_backend->free_trace_wait_data) {
		cur_tracing_backend->free_trace_wait_data(wd);
	} else {
		/* Default implementation: naked struct tcb_wait_data */
		free(wd);
	}
}

#endif /* !STRACE_TCB_WAIT_DATA_H */
