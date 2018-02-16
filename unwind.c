/*
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

#include "defs.h"
#include <libunwind-ptrace.h>

#ifdef USE_DEMANGLE
# if defined HAVE_DEMANGLE_H
#  include <demangle.h>
# elif defined HAVE_LIBIBERTY_DEMANGLE_H
#  include <libiberty/demangle.h>
# endif
#endif

/*
 * Type used in stacktrace walker
 */
typedef void (*call_action_fn)(void *data,
			       const char *binary_filename,
			       const char *symbol_name,
			       unw_word_t function_offset,
			       unsigned long true_offset);
typedef void (*error_action_fn)(void *data,
				const char *error,
				unsigned long true_offset);

/*
 * Type used in stacktrace capturing
 */
struct call_t {
	struct call_t *next;
	char *output_line;
};

struct queue_t {
	struct call_t *tail;
	struct call_t *head;
};

static void queue_print(struct queue_t *queue);

static unw_addr_space_t libunwind_as;

static const char asprintf_error_str[] = "???";

void
unwind_init(void)
{
	libunwind_as = unw_create_addr_space(&_UPT_accessors, 0);
	if (!libunwind_as)
		error_msg_and_die("failed to create address space for stack tracing");
	unw_set_caching_policy(libunwind_as, UNW_CACHE_GLOBAL);
}

void
unwind_tcb_init(struct tcb *tcp)
{
	if (tcp->libunwind_ui)
		return;

	tcp->libunwind_ui = _UPT_create(tcp->pid);
	if (!tcp->libunwind_ui)
		perror_msg_and_die("_UPT_create");

	tcp->queue = xmalloc(sizeof(*tcp->queue));
	tcp->queue->head = NULL;
	tcp->queue->tail = NULL;
}

void
unwind_tcb_fin(struct tcb *tcp)
{
	queue_print(tcp->queue);
	free(tcp->queue);
	tcp->queue = NULL;

	mmap_cache_delete(tcp, __func__);

	_UPT_destroy(tcp->libunwind_ui);
	tcp->libunwind_ui = NULL;
}

static void
get_symbol_name(unw_cursor_t *cursor, char **name,
		size_t *size, unw_word_t *offset)
{
	for (;;) {
		int rc = unw_get_proc_name(cursor, *name, *size, offset);
		if (rc == 0)
			break;
		if (rc != -UNW_ENOMEM) {
			**name = '\0';
			*offset = 0;
			break;
		}
		*name = xgrowarray(*name, size, 1);
	}
}

static int
print_stack_frame(struct tcb *tcp,
		  call_action_fn call_action,
		  error_action_fn error_action,
		  void *data,
		  unw_cursor_t *cursor,
		  char **symbol_name,
		  size_t *symbol_name_size)
{
	unw_word_t ip;
	struct mmap_cache_t *cur_mmap_cache;

	if (unw_get_reg(cursor, UNW_REG_IP, &ip) < 0) {
		perror_msg("Can't walk the stack of process %d", tcp->pid);
		return -1;
	}

	cur_mmap_cache = mmap_cache_search(tcp, ip);
	if (cur_mmap_cache
	    /* ignore mappings that have no PROT_EXEC bit set */
	    && (cur_mmap_cache->protections & MMAP_CACHE_PROT_EXECUTABLE)) {
		unsigned long true_offset;
		unw_word_t function_offset;

		get_symbol_name(cursor, symbol_name, symbol_name_size,
				&function_offset);
		true_offset = ip - cur_mmap_cache->start_addr +
			cur_mmap_cache->mmap_offset;
#ifdef USE_DEMANGLE
		char *demangled_name =
			cplus_demangle(*symbol_name,
				       DMGL_AUTO | DMGL_PARAMS);
#endif
		call_action(data,
			    cur_mmap_cache->binary_filename,
#ifdef USE_DEMANGLE
			    demangled_name ? demangled_name :
#endif
			    *symbol_name,
			    function_offset,
			    true_offset);
#ifdef USE_DEMANGLE
		free(demangled_name);
#endif

		return 0;
	}

	/*
	 * there is a bug in libunwind >= 1.0
	 * after a set_tid_address syscall
	 * unw_get_reg returns IP == 0
	 */
	if (ip)
		error_action(data, "unexpected_backtracing_error", ip);
	return -1;
}

/*
 * walking the stack
 */
static void
stacktrace_walk(struct tcb *tcp,
		call_action_fn call_action,
		error_action_fn error_action,
		void *data)
{
	char *symbol_name;
	size_t symbol_name_size = 40;
	unw_cursor_t cursor;
	int stack_depth;

	if (!tcp->mmap_cache)
		error_msg_and_die("bug: mmap_cache is NULL");
	if (tcp->mmap_cache_size == 0)
		error_msg_and_die("bug: mmap_cache is empty");

	symbol_name = xmalloc(symbol_name_size);

	if (unw_init_remote(&cursor, libunwind_as, tcp->libunwind_ui) < 0)
		perror_msg_and_die("Can't initiate libunwind");

	for (stack_depth = 0; stack_depth < 256; ++stack_depth) {
		if (print_stack_frame(tcp, call_action, error_action, data,
				&cursor, &symbol_name, &symbol_name_size) < 0)
			break;
		if (unw_step(&cursor) <= 0)
			break;
	}
	if (stack_depth >= 256)
		error_action(data, "too many stack frames", 0);

	free(symbol_name);
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
#define STACK_ENTRY_SYMBOL_FMT			\
	" > %s(%s+0x%lx) [0x%lx]\n",		\
	binary_filename,			\
	symbol_name,				\
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
	      unw_word_t function_offset,
	      unsigned long true_offset)
{
	if (symbol_name && (symbol_name[0] != '\0'))
		tprintf(STACK_ENTRY_SYMBOL_FMT);
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
		     unw_word_t function_offset,
		     unsigned long true_offset,
		     const char *error)
{
	char *output_line = NULL;
	int n;

	if (symbol_name)
		n = asprintf(&output_line, STACK_ENTRY_SYMBOL_FMT);
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
queue_put(struct queue_t *queue,
	  const char *binary_filename,
	  const char *symbol_name,
	  unw_word_t function_offset,
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
	       unw_word_t function_offset,
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
queue_print(struct queue_t *queue)
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
unwind_print_stacktrace(struct tcb *tcp)
{
#if SUPPORTED_PERSONALITIES > 1
	if (tcp->currpers != DEFAULT_PERSONALITY) {
		/* disable stack trace */
		return;
	}
#endif
	if (tcp->queue->head) {
		debug_func_msg("head: tcp=%p, queue=%p", tcp, tcp->queue->head);
		queue_print(tcp->queue);
	} else switch (mmap_cache_rebuild_if_invalid(tcp, __func__)) {
		case MMAP_CACHE_REBUILD_RENEWED:
			unw_flush_cache(libunwind_as, 0, 0);
			/* Fall through */
		case MMAP_CACHE_REBUILD_READY:
			debug_func_msg("walk: tcp=%p, queue=%p", tcp, tcp->queue->head);
			stacktrace_walk(tcp, print_call_cb, print_error_cb, NULL);
			break;
		default:
			/* Do nothing */
			;
	}
}

/*
 * capturing stack
 */
void
unwind_capture_stacktrace(struct tcb *tcp)
{
#if SUPPORTED_PERSONALITIES > 1
	if (tcp->currpers != DEFAULT_PERSONALITY) {
		/* disable stack trace */
		return;
	}
#endif
	if (tcp->queue->head)
		error_msg_and_die("bug: unprinted entries in queue");

	switch (mmap_cache_rebuild_if_invalid(tcp, __func__)) {
	case MMAP_CACHE_REBUILD_RENEWED:
		unw_flush_cache(libunwind_as, 0, 0);
		/* Fall through */
	case MMAP_CACHE_REBUILD_READY:
		stacktrace_walk(tcp, queue_put_call, queue_put_error,
				tcp->queue);
		debug_func_msg("tcp=%p, queue=%p", tcp, tcp->queue->head);
		break;
	default:
		/* Do nothing */
		;
	}
}
