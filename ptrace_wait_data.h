/*
 * Variant of wait data structure used by ptrace(2)-baed tracing backend.
 *
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_PTRACE_WAIT_DATA_H
#define STRACE_PTRACE_WAIT_DATA_H

#include "defs.h"
#include "tcb_wait_data.h"

/** ptrace-specific trace data */
struct ptrace_wait_data {
	struct tcb_wait_data wd; /**< Backend-agnostic data. */
	unsigned int status;     /**< status, as returned by wait4(). */
	unsigned int restart_op; /**< ptrace operation to restart tracee. */
	unsigned long msg;       /**< Value returned by PTRACE_GETEVENTMSG. */
};

#define to_ptrace_wait_data(p_) containerof((p_), struct ptrace_wait_data, wd)

#endif /* !STRACE_PTRACE_WAIT_DATA_H */
