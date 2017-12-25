/*
 * ptrace(2)-based backend interface.
 *
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_PTRACE_BACKEND_H
#define STRACE_PTRACE_BACKEND_H

#include "defs.h"
#include "trace_event.h"

extern bool ptrace_init(int argc, char *argv[]);

extern void ptrace_startup_child(char **argv, char **env);
extern void ptrace_attach_tcb(struct tcb *const tcp);
extern void ptrace_detach(struct tcb *tcp);

extern struct tcb_wait_data *ptrace_init_trace_wait_data(void *p);
extern struct tcb_wait_data *ptrace_copy_trace_wait_data(struct
							 tcb_wait_data *wd);
extern void ptrace_free_trace_wait_data(struct tcb_wait_data *wd);

extern struct tcb_wait_data *ptrace_next_event(void);
extern void ptrace_handle_exec(struct tcb **current_tcp,
			       struct tcb_wait_data *wd);
extern bool ptrace_restart_process(struct tcb *current_tcp,
				   struct tcb_wait_data *wd);

extern void ptrace_clear_regs(struct tcb *tcp);
extern long ptrace_get_regs(struct tcb * const tcp);
extern int ptrace_get_scno(struct tcb *tcp);
extern int ptrace_set_scno(struct tcb *tcp, kernel_ulong_t scno);
extern void ptrace_set_error(struct tcb *tcp, unsigned long new_error);
extern void ptrace_set_success(struct tcb *tcp, kernel_long_t new_rval);
extern bool ptrace_get_instruction_pointer(struct tcb *tcp, kernel_ulong_t *ip);
extern bool ptrace_get_stack_pointer(struct tcb *tcp, kernel_ulong_t *sp);
extern int ptrace_get_syscall_args(struct tcb *tcp);
extern int ptrace_get_syscall_result(struct tcb *tcp);

extern int ptrace_umoven(struct tcb *const tcp, kernel_ulong_t addr,
			 unsigned int len, void *const our_addr);
extern int ptrace_umovestr(struct tcb *const tcp, kernel_ulong_t addr,
			   unsigned int len, char *laddr);
extern int ptrace_upeek(struct tcb *tcp, unsigned long off,
			kernel_ulong_t *res);
extern int ptrace_upoke(struct tcb *tcp, unsigned long off, kernel_ulong_t val);

extern const struct tracing_backend ptrace_backend;

#endif /* !STRACE_PTRACE_BACKEND_H */
