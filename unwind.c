/*
 * Copyright (c) 2013 Luca Clementi <luca.clementi@gmail.com>
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

/*
 * Кeep a sorted array of cache entries,
 * so that we can binary search through it.
 */
struct mmap_cache_t {
	/**
	 * example entry:
	 * 7fabbb09b000-7fabbb09f000 r--p 00179000 fc:00 1180246 /lib/libc-2.11.1.so
	 *
	 * start_addr  is 0x7fabbb09b000
	 * end_addr    is 0x7fabbb09f000
	 * mmap_offset is 0x179000
	 * binary_filename is "/lib/libc-2.11.1.so"
	 */
	unsigned long start_addr;
	unsigned long end_addr;
	unsigned long mmap_offset;
	char* binary_filename;
};

static unw_addr_space_t libunwind_as;

void
unwind_init(void)
{
	libunwind_as = unw_create_addr_space(&_UPT_accessors, 0);
	if (!libunwind_as)
		error_msg_and_die("failed to create address space for stack tracing");
}

void
unwind_tcb_init(struct tcb *tcp)
{
	tcp->libunwind_ui = _UPT_create(tcp->pid);
	if (!tcp->libunwind_ui)
		die_out_of_memory();
}

void
unwind_tcb_fin(struct tcb *tcp)
{
	unwind_cache_invalidate(tcp);
	_UPT_destroy(tcp->libunwind_ui);
	tcp->libunwind_ui = NULL;
}

/*
 * caching of /proc/ID/maps for each process to speed up stack tracing
 *
 * The cache must be refreshed after some syscall: mmap, mprotect, munmap, execve
 */
static void
alloc_mmap_cache(struct tcb* tcp)
{
	unsigned long start_addr, end_addr, mmap_offset;
	char filename[sizeof ("/proc/0123456789/maps")];
	char buffer[PATH_MAX + 80];
	char binary_path[PATH_MAX];
	struct mmap_cache_t *cur_entry, *prev_entry;
	/* start with a small dynamically-allocated array and then expand it */
	size_t cur_array_size = 10;
	struct mmap_cache_t *cache_head;
	FILE *fp;

	sprintf(filename, "/proc/%d/maps", tcp->pid);
	fp = fopen(filename, "r");
	if (!fp) {
		perror_msg("fopen: %s", filename);
		return;
	}

	cache_head = calloc(cur_array_size, sizeof(*cache_head));
	if (!cache_head)
		die_out_of_memory();

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		binary_path[0] = '\0'; // 'reset' it just to be paranoid

		sscanf(buffer, "%lx-%lx %*c%*c%*c%*c %lx %*x:%*x %*d %[^\n]",
		       &start_addr, &end_addr, &mmap_offset, binary_path);

		/* ignore special 'fake files' like "[vdso]", "[heap]", "[stack]", */
		if (binary_path[0] == '[') {
			continue;
		}

		if (binary_path[0] == '\0') {
			continue;
		}

		if (end_addr < start_addr)
			perror_msg_and_die("%s: unrecognized maps file format",
					   filename);

		cur_entry = &cache_head[tcp->mmap_cache_size];
		cur_entry->start_addr = start_addr;
		cur_entry->end_addr = end_addr;
		cur_entry->mmap_offset = mmap_offset;
		cur_entry->binary_filename = strdup(binary_path);

		/*
		 * sanity check to make sure that we're storing
		 * non-overlapping regions in ascending order
		 */
		if (tcp->mmap_cache_size > 0) {
			prev_entry = &cache_head[tcp->mmap_cache_size - 1];
			if (prev_entry->start_addr >= cur_entry->start_addr)
				perror_msg_and_die("Overlaying memory region in %s",
						   filename);
			if (prev_entry->end_addr > cur_entry->start_addr)
				perror_msg_and_die("Overlaying memory region in %s",
						   filename);
		}
		tcp->mmap_cache_size++;

		/* resize doubling its size */
		if (tcp->mmap_cache_size >= cur_array_size) {
			cur_array_size *= 2;
			cache_head = realloc(cache_head, cur_array_size * sizeof(*cache_head));
			if (!cache_head)
				die_out_of_memory();
		}
	}
	fclose(fp);
	tcp->mmap_cache = cache_head;
}

/* deleting the cache */
void
unwind_cache_invalidate(struct tcb* tcp)
{
	unsigned int i;
	for (i = 0; i < tcp->mmap_cache_size; i++) {
		free(tcp->mmap_cache[i].binary_filename);
		tcp->mmap_cache[i].binary_filename = NULL;
	}
	free(tcp->mmap_cache);
	tcp->mmap_cache = NULL;
	tcp->mmap_cache_size = 0;
}

/* use libunwind to unwind the stack and print a backtrace */
void
unwind_print_stacktrace(struct tcb* tcp)
{
	unw_word_t ip;
	unw_cursor_t cursor;
	unw_word_t function_off_set;
	int stack_depth = 0, ret_val;
	/* these are used for the binary search through the mmap_chace */
	unsigned int lower, upper, mid;
	size_t symbol_name_size = 40;
	char * symbol_name;
	struct mmap_cache_t* cur_mmap_cache;
	unsigned long true_offset;

	if (!tcp->mmap_cache)
		alloc_mmap_cache(tcp);
	if (!tcp->mmap_cache || !tcp->mmap_cache_size)
		return;

	symbol_name = malloc(symbol_name_size);
	if (!symbol_name)
		die_out_of_memory();

	if (unw_init_remote(&cursor, libunwind_as, tcp->libunwind_ui) < 0)
		perror_msg_and_die("Can't initiate libunwind");

	do {
		/* looping on the stack frame */
		if (unw_get_reg(&cursor, UNW_REG_IP, &ip) < 0) {
			perror_msg("Can't walk the stack of process %d", tcp->pid);
			break;
		}

		lower = 0;
		upper = tcp->mmap_cache_size - 1;

		while (lower <= upper) {
			/* find the mmap_cache and print the stack frame */
			mid = (upper + lower) / 2;
			cur_mmap_cache = &tcp->mmap_cache[mid];

			if (ip >= cur_mmap_cache->start_addr &&
			    ip < cur_mmap_cache->end_addr) {
				for (;;) {
					symbol_name[0] = '\0';
					ret_val = unw_get_proc_name(&cursor, symbol_name,
						symbol_name_size, &function_off_set);
					if (ret_val != -UNW_ENOMEM)
						break;
					symbol_name_size *= 2;
					symbol_name = realloc(symbol_name, symbol_name_size);
					if (!symbol_name)
						die_out_of_memory();
				}

				true_offset = ip - cur_mmap_cache->start_addr +
					cur_mmap_cache->mmap_offset;
				if (symbol_name[0]) {
					/*
					 * we want to keep the format used by backtrace_symbols from the glibc
					 *
					 * ./a.out() [0x40063d]
					 * ./a.out() [0x4006bb]
					 * ./a.out() [0x4006c6]
					 * /lib64/libc.so.6(__libc_start_main+0xed) [0x7fa2f8a5976d]
					 * ./a.out() [0x400569]
					 */
					tprintf(" > %s(%s+0x%lx) [0x%lx]\n",
						cur_mmap_cache->binary_filename,
						symbol_name, function_off_set, true_offset);
				} else {
					tprintf(" > %s() [0x%lx]\n",
						cur_mmap_cache->binary_filename, true_offset);
				}
				line_ended();
				break; /* stack frame printed */
			}
			else if (mid == 0) {
				/*
				 * there is a bug in libunwind >= 1.0
				 * after a set_tid_address syscall
				 * unw_get_reg returns IP == 0
				 */
				if(ip)
					tprintf(" > backtracing_error\n");
				line_ended();
				goto ret;
			}
			else if (ip < cur_mmap_cache->start_addr)
				upper = mid;
			else
				lower = mid + 1;

		}
		if (lower > upper) {
			tprintf(" > backtracing_error [0x%lx]\n", ip);
			line_ended();
			goto ret;
		}

		ret_val = unw_step(&cursor);

		if (++stack_depth > 255) {
			tprintf("> too many stack frames\n");
			line_ended();
			break;
		}
	} while (ret_val > 0);
ret:
	free(symbol_name);
}
