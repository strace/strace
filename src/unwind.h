/*
 * Unwinder backends interface.
 *
 * Copyright (c) 2013 Luca Clementi <luca.clementi@gmail.com>
 * Copyright (c) 2013-2021 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_UNWIND_H
# define STRACE_UNWIND_H

# include "defs.h"

/*
 * Type used in stacktrace walker.
 */

/* This storage be enough large to store unw_word_t. */
typedef unsigned long unwind_function_offset_t;

typedef void (*unwind_call_action_fn)(void *data,
				      const char *binary_filename,
				      const char *symbol_name,
				      unwind_function_offset_t function_offset,
				      unsigned long true_offset);
typedef void (*unwind_error_action_fn)(void *data,
				       const char *error,
				       unsigned long true_offset);

struct unwind_unwinder_t {
	const char *name;

	/* Initialize the unwinder. */
	void   (*init)(void);

	/* Make/destroy the context data attached to tcb. */
	void * (*tcb_init)(struct tcb *);
	void   (*tcb_fin)(struct tcb *);

	/* Walk the stack. */
	void   (*tcb_walk)(struct tcb *,
			   unwind_call_action_fn,
			   unwind_error_action_fn,
			   void *);
};

extern const struct unwind_unwinder_t unwinder;

#endif /* !STRACE_UNWIND_H */
