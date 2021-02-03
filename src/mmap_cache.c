/*
 * Copyright (c) 2013 Luca Clementi <luca.clementi@gmail.com>
 * Copyright (c) 2013-2020 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <limits.h>

#include "largefile_wrappers.h"
#include "mmap_cache.h"
#include "mmap_notify.h"
#include "xstring.h"

static unsigned int mmap_cache_generation;

static void
mmap_cache_invalidate(struct tcb *tcp, void *unused)
{
	mmap_cache_generation++;
	debug_func_msg("tgen=%u, ggen=%u, tcp=%p, cache=%p",
		       tcp->mmap_cache ? tcp->mmap_cache->generation : 0,
		       mmap_cache_generation, tcp,
		       tcp->mmap_cache ? tcp->mmap_cache->entry : 0);
}

void
mmap_cache_enable(void)
{
	static bool use_mmap_cache;

	if (!use_mmap_cache) {
		mmap_notify_register_client(mmap_cache_invalidate, NULL);
		use_mmap_cache = true;
	}
}

/* deleting the cache */
static void
delete_mmap_cache(struct tcb *tcp, const char *caller)
{
	debug_func_msg("tgen=%u, ggen=%u, tcp=%p, cache=%p, caller=%s",
		       tcp->mmap_cache ? tcp->mmap_cache->generation : 0,
		       mmap_cache_generation, tcp,
		       tcp->mmap_cache ? tcp->mmap_cache->entry : 0, caller);

	if (!tcp->mmap_cache)
		return;

	while (tcp->mmap_cache->size) {
		unsigned int i = --tcp->mmap_cache->size;
		free(tcp->mmap_cache->entry[i].binary_filename);
		tcp->mmap_cache->entry[i].binary_filename = NULL;
	}

	free(tcp->mmap_cache->entry);
	tcp->mmap_cache->entry = NULL;

	free(tcp->mmap_cache);
	tcp->mmap_cache = NULL;
}

/*
 * caching of /proc/ID/maps for each process to speed up stack tracing
 *
 * The cache must be refreshed after syscalls that affect memory mappings,
 * e.g. mmap, mprotect, munmap, execve.
 */
extern enum mmap_cache_rebuild_result
mmap_cache_rebuild_if_invalid(struct tcb *tcp, const char *caller)
{
	if (tcp->mmap_cache
	    && tcp->mmap_cache->generation != mmap_cache_generation)
		delete_mmap_cache(tcp, caller);

	if (tcp->mmap_cache)
		return MMAP_CACHE_REBUILD_READY;

	char filename[sizeof("/proc/4294967296/maps")];
	xsprintf(filename, "/proc/%u/maps", get_proc_pid(tcp));

	FILE *fp = fopen_stream(filename, "r");
	if (!fp) {
		perror_msg("fopen: %s", filename);
		return MMAP_CACHE_REBUILD_NOCACHE;
	}

	struct mmap_cache_t cache = {
		.free_fn = delete_mmap_cache,
		.generation = mmap_cache_generation
	};

	/* start with a small dynamically-allocated array and then expand it */
	size_t allocated = 0;
	char buffer[PATH_MAX + 80];

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
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

		struct mmap_cache_entry_t *entry;
		/*
		 * sanity check to make sure that we're storing
		 * non-overlapping regions in ascending order
		 */
		if (cache.size > 0) {
			entry = &cache.entry[cache.size - 1];
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

		if (cache.size >= allocated)
			cache.entry = xgrowarray(cache.entry, &allocated,
						 sizeof(*cache.entry));

		entry = &cache.entry[cache.size];
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
		cache.size++;
	}
	fclose(fp);

	if (!cache.size)
		return MMAP_CACHE_REBUILD_NOCACHE;

	tcp->mmap_cache = xmalloc(sizeof(*tcp->mmap_cache));
	memcpy(tcp->mmap_cache, &cache, sizeof(cache));

	debug_func_msg("tgen=%u, ggen=%u, tcp=%p, cache=%p, caller=%s",
		       tcp->mmap_cache->generation, mmap_cache_generation,
		       tcp, tcp->mmap_cache->entry, caller);

	return MMAP_CACHE_REBUILD_RENEWED;
}

struct mmap_cache_entry_t *
mmap_cache_search(struct tcb *tcp, unsigned long ip)
{
	if (!tcp->mmap_cache)
		return NULL;

	int lower = 0;
	int upper = (int) tcp->mmap_cache->size - 1;

	while (lower <= upper) {
		int mid = (upper + lower) / 2;
		struct mmap_cache_entry_t *entry = &tcp->mmap_cache->entry[mid];

		if (ip >= entry->start_addr &&
		    ip < entry->end_addr)
			return entry;
		else if (ip < entry->start_addr)
			upper = mid - 1;
		else
			lower = mid + 1;
	}
	return NULL;
}

struct mmap_cache_entry_t *
mmap_cache_search_custom(struct tcb *tcp, mmap_cache_search_fn fn, void *data)
{
	for (unsigned int i = 0; i < tcp->mmap_cache->size; i++) {
		if (fn(tcp->mmap_cache->entry + i, data))
			return tcp->mmap_cache->entry + i;
	}
	return NULL;
}
