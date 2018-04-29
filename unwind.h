/*
 * Unwinder backends interface.
 *
 * Copyright (c) 2013 Luca Clementi <luca.clementi@gmail.com>
 * Copyright (c) 2013-2018 The strace developers.
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

#ifndef STRACE_UNWIND_H
#define STRACE_UNWIND_H

#include "defs.h"

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
