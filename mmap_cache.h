/*
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

#ifndef STRACE_MMAP_CACHE_H
#define STRACE_MMAP_CACHE_H

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
