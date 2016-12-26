/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2000 PocketPenguins Inc.  Linux for Hitachi SuperH
 *                    port by Greg Banks <gbanks@pocketpenguins.com>
 * All rights reserved.
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
#include <asm/mman.h>
#include <sys/mman.h>

unsigned long
get_pagesize(void)
{
	static unsigned long pagesize;

	if (!pagesize)
		pagesize = sysconf(_SC_PAGESIZE);
	return pagesize;
}

SYS_FUNC(brk)
{
	printaddr(tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_HEX;
}

#include "xlat/mmap_prot.h"
#include "xlat/mmap_flags.h"

static void
print_mmap(struct tcb *tcp, kernel_ulong_t *u_arg, unsigned long long offset)
{
	const kernel_ulong_t addr = u_arg[0];
	const kernel_ulong_t len = u_arg[1];
	const kernel_ulong_t prot = u_arg[2];
	const kernel_ulong_t flags = u_arg[3];
	const int fd = u_arg[4];

	printaddr(addr);
	tprintf(", %" PRI_klu ", ", len);
	printflags64(mmap_prot, prot, "PROT_???");
	tprints(", ");
#ifdef MAP_TYPE
	printxval64(mmap_flags, flags & MAP_TYPE, "MAP_???");
	addflags(mmap_flags, flags & ~MAP_TYPE);
#else
	printflags64(mmap_flags, flags, "MAP_???");
#endif
	tprints(", ");
	printfd(tcp, fd);
	tprintf(", %#llx", offset);
}

/* Syscall name<->function correspondence is messed up on many arches.
 * For example:
 * i386 has __NR_mmap == 90, and it is "old mmap", and
 * also it has __NR_mmap2 == 192, which is a "new mmap with page offsets".
 * But x86_64 has just one __NR_mmap == 9, a "new mmap with byte offsets".
 * Confused? Me too!
 */

#if defined AARCH64 || defined ARM \
 || defined I386 || defined X86_64 || defined X32 \
 || defined M68K \
 || defined S390 || defined S390X
/* Params are pointed to by u_arg[0], offset is in bytes */
SYS_FUNC(old_mmap)
{
	kernel_ulong_t u_arg[6];
# if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
	/* We are here only in a 32-bit personality. */
	unsigned int narrow_arg[6];
	if (umove_or_printaddr(tcp, tcp->u_arg[0], &narrow_arg))
		return RVAL_DECODED | RVAL_HEX;
	unsigned int i;
	for (i = 0; i < 6; i++)
		u_arg[i] = narrow_arg[i];
# else
	if (umove_or_printaddr(tcp, tcp->u_arg[0], &u_arg))
		return RVAL_DECODED | RVAL_HEX;
# endif
	print_mmap(tcp, u_arg, u_arg[5]);

	return RVAL_DECODED | RVAL_HEX;
}
#endif /* old_mmap architectures */

#ifdef S390
/* Params are pointed to by u_arg[0], offset is in pages */
SYS_FUNC(old_mmap_pgoff)
{
	kernel_ulong_t u_arg[5];
	int i;
	unsigned int narrow_arg[6];
	unsigned long long offset;
	if (umove_or_printaddr(tcp, tcp->u_arg[0], &narrow_arg))
		return RVAL_DECODED | RVAL_HEX;
	for (i = 0; i < 5; i++)
		u_arg[i] = narrow_arg[i];
	offset = narrow_arg[5];
	offset *= get_pagesize();
	print_mmap(tcp, u_arg, offset);

	return RVAL_DECODED | RVAL_HEX;
}
#endif /* S390 */

/* Params are passed directly, offset is in bytes */
SYS_FUNC(mmap)
{
	/* Example of kernel-side handling of this variety of mmap:
	 * arch/x86/kernel/sys_x86_64.c::SYSCALL_DEFINE6(mmap, ...) calls
	 * sys_mmap_pgoff(..., off >> PAGE_SHIFT); i.e. off is in bytes,
	 * since the above code converts off to pages.
	 */
	print_mmap(tcp, tcp->u_arg, tcp->u_arg[5]);

	return RVAL_DECODED | RVAL_HEX;
}

/* Params are passed directly, offset is in pages */
SYS_FUNC(mmap_pgoff)
{
	/* Try test/mmap_offset_decode.c */
	unsigned long long offset;
	offset = tcp->u_arg[5];
	offset *= get_pagesize();
	print_mmap(tcp, tcp->u_arg, offset);

	return RVAL_DECODED | RVAL_HEX;
}

/* Params are passed directly, offset is in 4k units */
SYS_FUNC(mmap_4koff)
{
	unsigned long long offset;
	offset = tcp->u_arg[5];
	offset <<= 12;
	print_mmap(tcp, tcp->u_arg, offset);

	return RVAL_DECODED | RVAL_HEX;
}

SYS_FUNC(munmap)
{
	printaddr(tcp->u_arg[0]);
	tprintf(", %" PRI_klu, tcp->u_arg[1]);

	return RVAL_DECODED;
}

static int
do_mprotect(struct tcb *tcp, bool has_pkey)
{
	printaddr(tcp->u_arg[0]);
	tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);
	printflags64(mmap_prot, tcp->u_arg[2], "PROT_???");

	if (has_pkey)
		tprintf(", %d", (int) tcp->u_arg[3]);

	return RVAL_DECODED;
}

SYS_FUNC(mprotect)
{
	return do_mprotect(tcp, false);
}

SYS_FUNC(pkey_mprotect)
{
	return do_mprotect(tcp, true);
}

#include "xlat/mremap_flags.h"

SYS_FUNC(mremap)
{
	printaddr(tcp->u_arg[0]);
	tprintf(", %" PRI_klu ", %" PRI_klu ", ", tcp->u_arg[1], tcp->u_arg[2]);
	printflags64(mremap_flags, tcp->u_arg[3], "MREMAP_???");
#ifdef MREMAP_FIXED
	if ((tcp->u_arg[3] & (MREMAP_MAYMOVE | MREMAP_FIXED)) ==
	    (MREMAP_MAYMOVE | MREMAP_FIXED)) {
		tprints(", ");
		printaddr(tcp->u_arg[4]);
	}
#endif
	return RVAL_DECODED | RVAL_HEX;
}

#include "xlat/madvise_cmds.h"

SYS_FUNC(madvise)
{
	printaddr(tcp->u_arg[0]);
	tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);
	printxval(madvise_cmds, tcp->u_arg[2], "MADV_???");

	return RVAL_DECODED;
}

#include "xlat/mlockall_flags.h"

SYS_FUNC(mlockall)
{
	printflags(mlockall_flags, tcp->u_arg[0], "MCL_???");

	return RVAL_DECODED;
}

#include "xlat/mctl_sync.h"

SYS_FUNC(msync)
{
	/* addr */
	printaddr(tcp->u_arg[0]);
	/* len */
	tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);
	/* flags */
	printflags(mctl_sync, tcp->u_arg[2], "MS_???");

	return RVAL_DECODED;
}

#include "xlat/mlock_flags.h"

SYS_FUNC(mlock2)
{
	printaddr(tcp->u_arg[0]);
	tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);
	printflags(mlock_flags, tcp->u_arg[2], "MLOCK_???");

	return RVAL_DECODED;
}

SYS_FUNC(mincore)
{
	if (entering(tcp)) {
		printaddr(tcp->u_arg[0]);
		tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);
	} else {
		const unsigned long page_size = get_pagesize();
		const unsigned long page_mask = page_size - 1;
		unsigned long len = tcp->u_arg[1];
		unsigned char *vec = NULL;

		len = len / page_size + (len & page_mask ? 1 : 0);
		if (syserror(tcp) || !verbose(tcp) ||
		    !tcp->u_arg[2] || !(vec = malloc(len)) ||
		    umoven(tcp, tcp->u_arg[2], len, vec) < 0)
			printaddr(tcp->u_arg[2]);
		else {
			unsigned long i;
			tprints("[");
			for (i = 0; i < len; i++) {
				if (i)
					tprints(", ");
				if (abbrev(tcp) && i >= max_strlen) {
					tprints("...");
					break;
				}
				tprints((vec[i] & 1) ? "1" : "0");
			}
			tprints("]");
		}
		free(vec);
	}
	return 0;
}

#if defined ALPHA || defined IA64 || defined M68K \
 || defined SPARC || defined SPARC64
SYS_FUNC(getpagesize)
{
	return RVAL_DECODED | RVAL_HEX;
}
#endif

SYS_FUNC(remap_file_pages)
{
	const kernel_ulong_t addr = tcp->u_arg[0];
	const kernel_ulong_t size = tcp->u_arg[1];
	const kernel_ulong_t prot = tcp->u_arg[2];
	const kernel_ulong_t pgoff = tcp->u_arg[3];
	const kernel_ulong_t flags = tcp->u_arg[4];

	printaddr(addr);
	tprintf(", %" PRI_klu ", ", size);
	printflags64(mmap_prot, prot, "PROT_???");
	tprintf(", %" PRI_klu ", ", pgoff);
#ifdef MAP_TYPE
	printxval64(mmap_flags, flags & MAP_TYPE, "MAP_???");
	addflags(mmap_flags, flags & ~MAP_TYPE);
#else
	printflags64(mmap_flags, flags, "MAP_???");
#endif

	return RVAL_DECODED;
}

#if defined(POWERPC)
static bool
print_protmap_entry(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	tprintf("%#08x", * (unsigned int *) elem_buf);

	return true;
}

SYS_FUNC(subpage_prot)
{
	kernel_ulong_t addr = tcp->u_arg[0];
	kernel_ulong_t len = tcp->u_arg[1];
	kernel_ulong_t nmemb = len >> 16;
	kernel_ulong_t map = tcp->u_arg[2];

	printaddr(addr);
	tprintf(", %" PRI_klu ", ", len);

	unsigned int entry;
	print_array(tcp, map, nmemb, &entry, sizeof(entry),
		    umoven_or_printaddr, print_protmap_entry, 0);

	return RVAL_DECODED;
}
#endif
