/*
 * Auxiliary children infrastructure support header.
 *
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_AUX_CHILDREN_H
#define STRACE_AUX_CHILDREN_H

#include <stdbool.h>

enum aux_child_ret {
	ACR_NO_ACTION,
	ACR_REMOVE_ME,
};

enum aux_child_sig {
	ACS_NONE,
	ACS_CONTINUE,
	ACS_TERMINATE,
};

typedef enum aux_child_sig (*aux_child_signal_fn)(pid_t pid, int status,
						  void *data);
typedef enum aux_child_ret (*aux_child_exit_fn)(pid_t pid, int exit_code,
						void *data);

/**
 * Structure containing handler functions that are called for an auxiliary
 * child process.
 */
struct aux_child_handlers {
	/**
	 * Callback that is called when an auxiliary child is signalled.
	 *
	 * It accepts pid of the auxiliary child that was signalled, status,
	 * as returned by the wait4() function, and data that is supplied in
	 * signal_fn_data argument of register_aux_child_ex function (or NULL,
	 * if the auxiliary child was registered with register_aux_child.
	 * It has to return one of the entities defined in enum aux_child_sig
	 * enumeration:
	 *  - ACR_NO_ACTION, if no action has to be performed.
	 *  - ACR_REMOVE_ME, if the auxiliary child has to be removed
	 *                   from the auxiliary children list.
	 *
	 * The default callback (that is, the function which is called when this
	 * field is set to NULL) is aux_child_sig_handler.
	 */
	aux_child_signal_fn signal_fn;
	/**
	 * Callback that is called when an auxiliary child has to be notified
	 * about strace's termination.
	 */
	aux_child_exit_fn exit_notify_fn;
	/**
	 * A callback that is used for waiting for an auxiliary child.
	 */
	aux_child_exit_fn exit_wait_fn;
};


/**
 * Register an auxiliary child process (that is, not a tracee).
 */
extern void register_aux_child_ex(pid_t pid, const struct aux_child_handlers *h,
				  void *signal_fn_data,
				  void *exit_notify_fn_data,
				  void *exit_wait_fn_data);
/* Do not remove other children from the aux_child handlers.  */
extern void remove_aux_child(pid_t pid);

/**
 * A shorthand for registering an aux child with default handlers.
 *
 * @param pid Child's pid.
 */
static inline void
register_aux_child(pid_t pid)
{
	register_aux_child_ex(pid, NULL, NULL, NULL, NULL);
}

extern bool have_aux_children(void);

extern enum aux_child_sig aux_children_signal(pid_t pid, int status);
extern void aux_children_exit_notify(int exit_code);
extern int aux_children_exit_wait(int exit_code);

/** Default signal handler, removes child from list */
extern enum aux_child_sig aux_child_sig_handler(pid_t pid, int status,
						void *data);
/** Default wait handler, waits for the child and then returns ACR_REMOVE_ME */
extern enum aux_child_ret aux_child_wait_handler(pid_t pid, int exit_code,
						 void *data);

#endif /* !STRACE_AUX_CHILDREN_H */
