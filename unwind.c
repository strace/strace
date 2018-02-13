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
#include <limits.h>
#include <libunwind-ptrace.h>

#ifdef USE_DEMANGLE
# if defined HAVE_DEMANGLE_H
#  include <demangle.h>
# elif defined HAVE_LIBIBERTY_DEMANGLE_H
#  include <libiberty/demangle.h>
# endif
#endif

#include "xstring.h"

#ifdef _LARGEFILE64_SOURCE
# ifdef HAVE_FOPEN64
#  define fopen_for_input fopen64
# else
#  define fopen_for_input fopen
# endif
#else
# define fopen_for_input fopen
#endif

/*
 * Keep a sorted array of cache entries,
 * so that we can binary search through it.
 */
struct mmap_cache_t {
	/**
	 * example entry:
	 * 7fabbb09b000-7fabbb09f000 r-xp 00179000 fc:00 1180246 /lib/libc-2.11.1.so
	 *
	 * start_addr  is 0x7fabbb09b000
	 * end_addr    is 0x7fabbb09f000
	 * mmap_offset is 0x179000
	 * binary_filename is "/lib/libc-2.11.1.so"
	 */
	unsigned long start_addr;
	unsigned long end_addr;
	unsigned long mmap_offset;
	char *binary_filename;
};

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
static void delete_mmap_cache(struct tcb *tcp, const char *caller);

static unw_addr_space_t libunwind_as;
static unsigned int mmap_cache_generation;

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

	delete_mmap_cache(tcp, __func__);

	_UPT_destroy(tcp->libunwind_ui);
	tcp->libunwind_ui = NULL;
}

/*
 * caching of /proc/ID/maps for each process to speed up stack tracing
 *
 * The cache must be refreshed after syscalls that affect memory mappings,
 * e.g. mmap, mprotect, munmap, execve.
 */
static void
build_mmap_cache(struct tcb *tcp)
{
	FILE *fp;
	struct mmap_cache_t *cache_head = NULL;
	size_t cur_array_size = 0;
	char filename[sizeof("/proc/4294967296/maps")];
	char buffer[PATH_MAX + 80];

	unw_flush_cache(libunwind_as, 0, 0);

	xsprintf(filename, "/proc/%u/maps", tcp->pid);
	fp = fopen_for_input(filename, "r");
	if (!fp) {
		perror_msg("fopen: %s", filename);
		return;
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		struct mmap_cache_t *entry;
		unsigned long start_addr, end_addr, mmap_offset;
		char exec_bit;
		char binary_path[sizeof(buffer)];

		if (sscanf(buffer, "%lx-%lx %*c%*c%c%*c %lx %*x:%*x %*d %[^\n]",
			   &start_addr, &end_addr, &exec_bit,
			   &mmap_offset, binary_path) != 5)
			continue;

		/* ignore mappings that have no PROT_EXEC bit set */
		if (exec_bit != 'x')
			continue;

		if (end_addr < start_addr) {
			error_msg("%s: unrecognized file format", filename);
			break;
		}

		/*
		 * sanity check to make sure that we're storing
		 * non-overlapping regions in ascending order
		 */
		if (tcp->mmap_cache_size > 0) {
			entry = &cache_head[tcp->mmap_cache_size - 1];
			if (entry->start_addr == start_addr &&
			    entry->end_addr == end_addr) {
				/* duplicate entry, e.g. [vsyscall] */
				continue;
			}
			if (start_addr <= entry->start_addr ||
			    start_addr < entry->end_addr) {
				debug_msg("%s: overlapping memory region: "
					  "\"%s\" [%08lx-%08lx] overlaps with "
					  "\"%s\" [%08lx-%08lx]",
					  filename, binary_path, start_addr,
					  end_addr, entry->binary_filename,
					  entry->start_addr, entry->end_addr);
				continue;
			}
		}

		if (tcp->mmap_cache_size >= cur_array_size)
			cache_head = xgrowarray(cache_head, &cur_array_size,
						sizeof(*cache_head));

		entry = &cache_head[tcp->mmap_cache_size];
		entry->start_addr = start_addr;
		entry->end_addr = end_addr;
		entry->mmap_offset = mmap_offset;
		entry->binary_filename = xstrdup(binary_path);
		tcp->mmap_cache_size++;
	}
	fclose(fp);
	tcp->mmap_cache = cache_head;
	tcp->mmap_cache_generation = mmap_cache_generation;

	debug_func_msg("tgen=%u, ggen=%u, tcp=%p, cache=%p",
		       tcp->mmap_cache_generation,
		       mmap_cache_generation,
		       tcp, tcp->mmap_cache);
}

/* deleting the cache */
static void
delete_mmap_cache(struct tcb *tcp, const char *caller)
{
	unsigned int i;

	debug_func_msg("tgen=%u, ggen=%u, tcp=%p, cache=%p, caller=%s",
		       tcp->mmap_cache_generation,
		       mmap_cache_generation,
		       tcp, tcp->mmap_cache, caller);

	for (i = 0; i < tcp->mmap_cache_size; i++) {
		free(tcp->mmap_cache[i].binary_filename);
		tcp->mmap_cache[i].binary_filename = NULL;
	}
	free(tcp->mmap_cache);
	tcp->mmap_cache = NULL;
	tcp->mmap_cache_size = 0;
}

static bool
rebuild_cache_if_invalid(struct tcb *tcp, const char *caller)
{
	if ((tcp->mmap_cache_generation != mmap_cache_generation)
	    && tcp->mmap_cache)
		delete_mmap_cache(tcp, caller);

	if (!tcp->mmap_cache)
		build_mmap_cache(tcp);

	return tcp->mmap_cache && tcp->mmap_cache_size;
}

void
unwind_cache_invalidate(struct tcb *tcp)
{
#if SUPPORTED_PERSONALITIES > 1
	if (tcp->currpers != DEFAULT_PERSONALITY) {
		/* disable stack trace */
		return;
	}
#endif
	mmap_cache_generation++;
	debug_func_msg("tgen=%u, ggen=%u, tcp=%p, cache=%p",
		       tcp->mmap_cache_generation,
		       mmap_cache_generation,
		       tcp, tcp->mmap_cache);
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
	int lower = 0;
	int upper = (int) tcp->mmap_cache_size - 1;

	if (unw_get_reg(cursor, UNW_REG_IP, &ip) < 0) {
		perror_msg("Can't walk the stack of process %d", tcp->pid);
		return -1;
	}

	while (lower <= upper) {
		struct mmap_cache_t *cur_mmap_cache;
		int mid = (upper + lower) / 2;

		cur_mmap_cache = &tcp->mmap_cache[mid];

		if (ip >= cur_mmap_cache->start_addr &&
		    ip < cur_mmap_cache->end_addr) {
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
		} else if (ip < cur_mmap_cache->start_addr)
			upper = mid - 1;
		else
			lower = mid + 1;
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
	} else if (rebuild_cache_if_invalid(tcp, __func__)) {
		debug_func_msg("walk: tcp=%p, queue=%p", tcp, tcp->queue->head);
		stacktrace_walk(tcp, print_call_cb, print_error_cb, NULL);
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

	if (rebuild_cache_if_invalid(tcp, __func__)) {
		stacktrace_walk(tcp, queue_put_call, queue_put_error,
				tcp->queue);
		debug_func_msg("tcp=%p, queue=%p", tcp, tcp->queue->head);
	}
}
