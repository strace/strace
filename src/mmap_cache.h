/*
 * Copyright (c) 2013-2021 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_MMAP_CACHE_H
# define STRACE_MMAP_CACHE_H

/*
 * Keep a sorted array of cache entries,
 * so that we can binary search through it.
 */

struct mmap_cache_t {
	struct mmap_cache_entry_t *entry;
	void (*free_fn)(struct tcb *, const char *caller);
	unsigned int size;
	unsigned int generation;
};

struct mmap_cache_entry_t {
	/**
	 * example entry:
	 * 7fabbb09b000-7fabbb09f000 r-xp 00179000 fc:00 1180246 /lib/libc-2.11.1.so
	 *
	 * start_addr  is 0x7fabbb09b000
	 * end_addr    is 0x7fabbb09f000
	 * mmap_offset is 0x179000
	 * protections is MMAP_CACHE_PROT_READABLE|MMAP_CACHE_PROT_EXECUTABLE
	 * major       is 0xfc
	 * minor       is 0x00
	 * binary_filename is "/lib/libc-2.11.1.so"
	 */
	unsigned long start_addr;
	unsigned long end_addr;
	unsigned long mmap_offset;
	unsigned char protections;
	unsigned long major, minor;
	char *binary_filename;
};

enum mmap_cache_protection {
	MMAP_CACHE_PROT_READABLE   = 1 << 0,
	MMAP_CACHE_PROT_WRITABLE   = 1 << 1,
	MMAP_CACHE_PROT_EXECUTABLE = 1 << 2,
	MMAP_CACHE_PROT_SHARED     = 1 << 3,
};

enum mmap_cache_rebuild_result {
	MMAP_CACHE_REBUILD_NOCACHE,
	MMAP_CACHE_REBUILD_READY,
	MMAP_CACHE_REBUILD_RENEWED,
};

typedef bool (*mmap_cache_search_fn)(struct mmap_cache_entry_t *, void *);

extern void
mmap_cache_enable(void);

extern enum mmap_cache_rebuild_result
mmap_cache_rebuild_if_invalid(struct tcb *, const char *caller);

extern struct mmap_cache_entry_t *
mmap_cache_search(struct tcb *, unsigned long ip);

extern struct mmap_cache_entry_t *
mmap_cache_search_custom(struct tcb *, mmap_cache_search_fn, void *);

#endif /* !STRACE_MMAP_CACHE_H */
