/*
 * Copyright (c) 2013 Luca Clementi <luca.clementi@gmail.com>
 * Copyright (c) 2013-2020 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "unwind.h"

#ifdef USE_DEMANGLE
# if defined HAVE_DEMANGLE_H
#  include <demangle.h>
# elif defined HAVE_LIBIBERTY_DEMANGLE_H
#  include <libiberty/demangle.h>
# endif
#endif

/*
 * Type used in stacktrace capturing
 */
struct call_t {
	struct call_t *next;
	char *output_line;
};

struct unwind_queue_t {
	struct call_t *tail;
	struct call_t *head;
};

static void queue_print(struct unwind_queue_t *queue);

static const char asprintf_error_str[] = "???";

void
unwind_init(void)
{
	if (unwinder.init)
		unwinder.init();
}

void
unwind_tcb_init(struct tcb *tcp)
{
	if (tcp->unwind_queue)
		return;

	tcp->unwind_queue = xmalloc(sizeof(*tcp->unwind_queue));
	tcp->unwind_queue->head = NULL;
	tcp->unwind_queue->tail = NULL;

	tcp->unwind_ctx = unwinder.tcb_init(tcp);
}

void
unwind_tcb_fin(struct tcb *tcp)
{
	if (!tcp->unwind_queue)
		return;

	queue_print(tcp->unwind_queue);
	free(tcp->unwind_queue);
	tcp->unwind_queue = NULL;

	unwinder.tcb_fin(tcp);
	tcp->unwind_ctx = NULL;
}

/*
 * printing an entry in stack to stream or buffer
 */
/*
 * we want to keep the format used by backtrace_symbols from the glibc
 *
 * ./a.out() [0x40063d]
 * ./a.out() [0x4006bb]
 * ./a.out() [0x4006c6]
 * /lib64/libc.so.6(__libc_start_main+0xed) [0x7fa2f8a5976d]
 * ./a.out() [0x400569]
 */
#define STACK_ENTRY_SYMBOL_FMT(SYM)		\
	" > %s(%s+0x%lx) [0x%lx]\n",		\
	binary_filename,			\
	(SYM),					\
	(unsigned long) function_offset,	\
	true_offset
#define STACK_ENTRY_NOSYMBOL_FMT		\
	" > %s() [0x%lx]\n",			\
	binary_filename, true_offset
#define STACK_ENTRY_BUG_FMT			\
	" > BUG IN %s\n"
#define STACK_ENTRY_ERROR_WITH_OFFSET_FMT	\
	" > %s [0x%lx]\n", error, true_offset
#define STACK_ENTRY_ERROR_FMT			\
	" > %s\n", error

static void
print_call_cb(void *dummy,
	      const char *binary_filename,
	      const char *symbol_name,
	      unwind_function_offset_t function_offset,
	      unsigned long true_offset)
{
	if (symbol_name && (symbol_name[0] != '\0')) {
#ifdef USE_DEMANGLE
		char *demangled_name =
			cplus_demangle(symbol_name,
				       DMGL_AUTO | DMGL_PARAMS);
#endif
		tprintf(STACK_ENTRY_SYMBOL_FMT(
#ifdef USE_DEMANGLE
					       demangled_name ? demangled_name :
#endif
					       symbol_name));
#ifdef USE_DEMANGLE
		free(demangled_name);
#endif
	}
	else if (binary_filename)
		tprintf(STACK_ENTRY_NOSYMBOL_FMT);
	else
		tprintf(STACK_ENTRY_BUG_FMT, __func__);

	line_ended();
}

static void
print_error_cb(void *dummy,
	       const char *error,
	       unsigned long true_offset)
{
	if (true_offset)
		tprintf(STACK_ENTRY_ERROR_WITH_OFFSET_FMT);
	else
		tprintf(STACK_ENTRY_ERROR_FMT);

	line_ended();
}

static char *
sprint_call_or_error(const char *binary_filename,
		     const char *symbol_name,
		     unwind_function_offset_t function_offset,
		     unsigned long true_offset,
		     const char *error)
{
	char *output_line = NULL;
	int n;

	if (symbol_name) {
#ifdef USE_DEMANGLE
		char *demangled_name =
			cplus_demangle(symbol_name,
				       DMGL_AUTO | DMGL_PARAMS);
#endif
		n = asprintf(&output_line,
			     STACK_ENTRY_SYMBOL_FMT(
#ifdef USE_DEMANGLE
						    demangled_name ? demangled_name :
#endif
						    symbol_name));
#ifdef USE_DEMANGLE
		free(demangled_name);
#endif
	}
	else if (binary_filename)
		n = asprintf(&output_line, STACK_ENTRY_NOSYMBOL_FMT);
	else if (error)
		n = true_offset
			? asprintf(&output_line, STACK_ENTRY_ERROR_WITH_OFFSET_FMT)
			: asprintf(&output_line, STACK_ENTRY_ERROR_FMT);
	else
		n = asprintf(&output_line, STACK_ENTRY_BUG_FMT, __func__);

	if (n < 0) {
		perror_func_msg("asprintf");
		output_line = (char *) asprintf_error_str;
	}

	return output_line;
}

/*
 * queue manipulators
 */
static void
queue_put(struct unwind_queue_t *queue,
	  const char *binary_filename,
	  const char *symbol_name,
	  unwind_function_offset_t function_offset,
	  unsigned long true_offset,
	  const char *error)
{
	struct call_t *call;

	call = xmalloc(sizeof(*call));
	call->output_line = sprint_call_or_error(binary_filename,
						 symbol_name,
						 function_offset,
						 true_offset,
						 error);
	call->next = NULL;

	if (!queue->head) {
		queue->head = call;
		queue->tail = call;
	} else {
		queue->tail->next = call;
		queue->tail = call;
	}
}

static void
queue_put_call(void *queue,
	       const char *binary_filename,
	       const char *symbol_name,
	       unwind_function_offset_t function_offset,
	       unsigned long true_offset)
{
	queue_put(queue,
		  binary_filename,
		  symbol_name,
		  function_offset,
		  true_offset,
		  NULL);
}

static void
queue_put_error(void *queue,
		const char *error,
		unsigned long ip)
{
	queue_put(queue, NULL, NULL, 0, ip, error);
}

static void
queue_print(struct unwind_queue_t *queue)
{
	struct call_t *call, *tmp;

	queue->tail = NULL;
	call = queue->head;
	queue->head = NULL;
	while (call) {
		tmp = call;
		call = call->next;

		tprints(tmp->output_line);
		line_ended();

		if (tmp->output_line != asprintf_error_str)
			free(tmp->output_line);

		tmp->output_line = NULL;
		tmp->next = NULL;
		free(tmp);
	}
}

/*
 * printing stack
 */
void
unwind_tcb_print(struct tcb *tcp)
{
#if defined(USE_LIBUNWIND) && (SUPPORTED_PERSONALITIES > 1)
	if (tcp->currpers != DEFAULT_PERSONALITY) {
		/* disable stack trace */
		return;
	}
#endif
	if (tcp->unwind_queue->head) {
		debug_func_msg("head: tcp=%p, queue=%p",
			       tcp, tcp->unwind_queue->head);
		queue_print(tcp->unwind_queue);
	} else
		unwinder.tcb_walk(tcp, print_call_cb, print_error_cb, NULL);
}

/*
 * capturing stack
 */
void
unwind_tcb_capture(struct tcb *tcp)
{
#if defined(USE_LIBUNWIND) && (SUPPORTED_PERSONALITIES > 1)
	if (tcp->currpers != DEFAULT_PERSONALITY) {
		/* disable stack trace */
		return;
	}
#endif
	if (tcp->unwind_queue->head)
		error_msg_and_die("bug: unprinted entries in queue");
	else {
		debug_func_msg("walk: tcp=%p, queue=%p",
			       tcp, tcp->unwind_queue->head);
		unwinder.tcb_walk(tcp, queue_put_call, queue_put_error,
				  tcp->unwind_queue);
	}
}
