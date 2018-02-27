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

#include "largefile_wrappers.h"
#include "mmap_cache.h"
#include "xstring.h"

static unsigned int mmap_cache_generation;
static bool use_mmap_cache;

extern void mmap_cache_enable(void)
{
	use_mmap_cache = true;
}

extern bool mmap_cache_is_enabled(void)
{
	return use_mmap_cache;
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
	/* start with a small dynamically-allocated array and then expand it */
	size_t cur_array_size = 0;
	char filename[sizeof("/proc/4294967296/maps")];
	char buffer[PATH_MAX + 80];

	xsprintf(filename, "/proc/%u/maps", tcp->pid);
	fp = fopen_stream(filename, "r");
	if (!fp) {
		perror_msg("fopen: %s", filename);
		return;
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		struct mmap_cache_t *entry;
		unsigned long start_addr, end_addr, mmap_offset;
		char read_bit;
		char write_bit;
		char exec_bit;
		char shared_bit;
		unsigned long major, minor;
		char binary_path[sizeof(buffer)];

		if (sscanf(buffer, "%lx-%lx %c%c%c%c %lx %lx:%lx %*d %[^\n]",
			   &start_addr, &end_addr,
			   &read_bit, &write_bit, &exec_bit, &shared_bit,
			   &mmap_offset,
			   &major, &minor,
			   binary_path) != 10)
			continue;

		/* skip mappings that have unknown protection */
		if (!(read_bit == '-' || read_bit == 'r'))
			continue;
		if (!(write_bit == '-' || write_bit == 'w'))
			continue;
		if (!(exec_bit == '-' || exec_bit == 'x'))
			continue;
		if (!(shared_bit == 'p' || shared_bit == 's'))
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
		entry->protections = (
			0
			| ((read_bit   == 'r')? MMAP_CACHE_PROT_READABLE  : 0)
			| ((write_bit  == 'w')? MMAP_CACHE_PROT_WRITABLE  : 0)
			| ((exec_bit   == 'x')? MMAP_CACHE_PROT_EXECUTABLE: 0)
			| ((shared_bit == 's')? MMAP_CACHE_PROT_SHARED    : 0)
			);
		entry->major = major;
		entry->minor = minor;
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
extern void
mmap_cache_delete(struct tcb *tcp, const char *caller)
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

extern enum mmap_cache_rebuild_result
mmap_cache_rebuild_if_invalid(struct tcb *tcp, const char *caller)
{
	enum mmap_cache_rebuild_result r = MMAP_CACHE_REBUILD_READY;
	if ((tcp->mmap_cache_generation != mmap_cache_generation)
	    && tcp->mmap_cache)
		mmap_cache_delete(tcp, caller);

	if (!tcp->mmap_cache) {
		r = MMAP_CACHE_REBUILD_RENEWED;
		build_mmap_cache(tcp);
	}

	if (!(tcp->mmap_cache && tcp->mmap_cache_size))
		r = MMAP_CACHE_REBUILD_NOCACHE;

	return r;
}

void
mmap_cache_invalidate(struct tcb *tcp)
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

struct mmap_cache_t *
mmap_cache_search(struct tcb *tcp, unsigned long ip)
{
	int lower = 0;
	int upper = (int) tcp->mmap_cache_size - 1;

	while (lower <= upper) {
		struct mmap_cache_t *cur_mmap_cache;
		int mid = (upper + lower) / 2;

		cur_mmap_cache = &tcp->mmap_cache[mid];

		if (ip >= cur_mmap_cache->start_addr &&
		    ip < cur_mmap_cache->end_addr)
			return cur_mmap_cache;
		else if (ip < cur_mmap_cache->start_addr)
			upper = mid - 1;
		else
			lower = mid + 1;
	}
	return NULL;
}
