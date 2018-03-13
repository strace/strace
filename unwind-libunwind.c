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
#include "unwind.h"

#include "mmap_cache.h"
#include <libunwind-ptrace.h>

static unw_addr_space_t libunwind_as;

static void
init(void)
{
	libunwind_as = unw_create_addr_space(&_UPT_accessors, 0);
	if (!libunwind_as)
		error_msg_and_die("failed to create address space"
				  " for stack tracing");
	unw_set_caching_policy(libunwind_as, UNW_CACHE_GLOBAL);
}

static void *
tcb_init(struct tcb *tcp)
{
	void *r = _UPT_create(tcp->pid);

	if (!r)
		perror_msg_and_die("_UPT_create");
	return r;
}

static void
tcb_fin(struct tcb *tcp)
{
	_UPT_destroy(tcp->unwind_ctx);
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
		  unwind_call_action_fn call_action,
		  unwind_error_action_fn error_action,
		  void *data,
		  unw_cursor_t *cursor,
		  char **symbol_name,
		  size_t *symbol_name_size)
{
	unw_word_t ip;
	struct mmap_cache_t *cur_mmap_cache;

	if (unw_get_reg(cursor, UNW_REG_IP, &ip) < 0) {
		perror_msg("cannot walk the stack of process %d", tcp->pid);
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
		call_action(data,
			    cur_mmap_cache->binary_filename,
			    *symbol_name,
			    function_offset,
			    true_offset);

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

static void
tcb_walk(struct tcb *tcp,
	 unwind_call_action_fn call_action,
	 unwind_error_action_fn error_action,
	 void *data)
{
	char *symbol_name;
	size_t symbol_name_size = 40;
	unw_cursor_t cursor;
	int stack_depth;

	if (!tcp->mmap_cache)
		error_func_msg_and_die("mmap_cache is NULL");
	if (tcp->mmap_cache_size == 0)
		error_func_msg_and_die("mmap_cache is empty");

	symbol_name = xmalloc(symbol_name_size);

	if (unw_init_remote(&cursor, libunwind_as, tcp->unwind_ctx) < 0)
		perror_func_msg_and_die("cannot initialize libunwind");

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

static void
tcb_flush_cache(struct tcb *tcp)
{
	unw_flush_cache(libunwind_as, 0, 0);
}

const struct unwind_unwinder_t unwinder = {
	.name = "libunwind",
	.init = init,
	.tcb_init = tcb_init,
	.tcb_fin = tcb_fin,
	.tcb_walk = tcb_walk,
	.tcb_flush_cache = tcb_flush_cache,
};
