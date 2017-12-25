/*
 * ptrace(2)-based-tracing-backend-specific definitions.
 *
 * Copyright (c) 2017 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "ptrace_backend.h"
#include "ptrace_wait_data.h"

const struct tracing_backend ptrace_backend = {
	.name                    = "ptrace",

	.init                    = ptrace_init,

	.startup_child           = ptrace_startup_child,
	.attach_tcb              = ptrace_attach_tcb,
	.detach                  = ptrace_detach,

	.trace_wait_data_size    = sizeof(struct ptrace_wait_data),
	.init_trace_wait_data    = ptrace_init_trace_wait_data,
	.copy_trace_wait_data    = ptrace_copy_trace_wait_data,
	.free_trace_wait_data    = ptrace_free_trace_wait_data,

	.next_event              = ptrace_next_event,
	.handle_exec             = ptrace_handle_exec,
	.restart_process         = ptrace_restart_process,

	.clear_regs              = ptrace_clear_regs,
	.get_regs                = ptrace_get_regs,
	.get_scno                = ptrace_get_scno,
	.set_scno                = ptrace_set_scno,
	.set_error               = ptrace_set_error,
	.set_success             = ptrace_set_success,
	.get_instruction_pointer = ptrace_get_instruction_pointer,
	.get_stack_pointer       = ptrace_get_stack_pointer,
	.get_syscall_args        = ptrace_get_syscall_args,
	.get_syscall_result      = ptrace_get_syscall_result,

	.umoven                  = ptrace_umoven,
	.umovestr                = ptrace_umovestr,
	.upeek                   = ptrace_upeek,
	.upoke                   = ptrace_upoke,

	/* ptrace(2)-based backend is always local */
	.kill                    = local_kill,
	.realpath                = local_realpath,
	.open                    = local_open,
	.pread                   = local_pread,
	.close                   = local_close,
	.readlink                = local_readlink,
	.stat                    = local_stat,
	.fstat                   = local_fstat,
	.getxattr                = local_getxattr,
	.socket                  = local_socket,
	.sendmsg                 = local_sendmsg,
	.recvmsg                 = local_recvmsg,
};
