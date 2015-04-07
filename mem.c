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
	if (entering(tcp)) {
		tprintf("%#lx", tcp->u_arg[0]);
	}
	return RVAL_HEX;
}

#include "xlat/mmap_prot.h"
#include "xlat/mmap_flags.h"

static int
print_mmap(struct tcb *tcp, long *u_arg, unsigned long long offset)
{
	if (entering(tcp)) {
		/* addr */
		if (!u_arg[0])
			tprints("NULL, ");
		else
			tprintf("%#lx, ", u_arg[0]);
		/* len */
		tprintf("%lu, ", u_arg[1]);
		/* prot */
		printflags(mmap_prot, u_arg[2], "PROT_???");
		tprints(", ");
		/* flags */
#ifdef MAP_TYPE
		printxval(mmap_flags, u_arg[3] & MAP_TYPE, "MAP_???");
		addflags(mmap_flags, u_arg[3] & ~MAP_TYPE);
#else
		printflags(mmap_flags, u_arg[3], "MAP_???");
#endif
		tprints(", ");
		/* fd */
		printfd(tcp, u_arg[4]);
		/* offset */
		tprintf(", %#llx", offset);
	}
	return RVAL_HEX;
}

/* Syscall name<->function correspondence is messed up on many arches.
 * For example:
 * i386 has __NR_mmap == 90, and it is "old mmap", and
 * also it has __NR_mmap2 == 192, which is a "new mmap with page offsets".
 * But x86_64 has just one __NR_mmap == 9, a "new mmap with byte offsets".
 * Confused? Me too!
 */

/* Params are pointed to by u_arg[0], offset is in bytes */
SYS_FUNC(old_mmap)
{
	long u_arg[6];
#if defined(IA64)
	/*
	 * IA64 processes never call this routine, they only use the
	 * new 'sys_mmap' interface. Only IA32 processes come here.
	 */
	int i;
	unsigned narrow_arg[6];
	if (umoven(tcp, tcp->u_arg[0], sizeof(narrow_arg), narrow_arg) == -1)
		return 0;
	for (i = 0; i < 6; i++)
		u_arg[i] = (unsigned long) narrow_arg[i];
#elif defined(X86_64)
	/* We are here only in personality 1 (i386) */
	int i;
	unsigned narrow_arg[6];
	if (umoven(tcp, tcp->u_arg[0], sizeof(narrow_arg), narrow_arg) == -1)
		return 0;
	for (i = 0; i < 6; ++i)
		u_arg[i] = (unsigned long) narrow_arg[i];
#else
	if (umoven(tcp, tcp->u_arg[0], sizeof(u_arg), u_arg) == -1)
		return 0;
#endif
	return print_mmap(tcp, u_arg, (unsigned long) u_arg[5]);
}

#if defined(S390)
/* Params are pointed to by u_arg[0], offset is in pages */
SYS_FUNC(old_mmap_pgoff)
{
	long u_arg[5];
	int i;
	unsigned narrow_arg[6];
	unsigned long long offset;
	if (umoven(tcp, tcp->u_arg[0], sizeof(narrow_arg), narrow_arg) == -1)
		return 0;
	for (i = 0; i < 5; i++)
		u_arg[i] = (unsigned long) narrow_arg[i];
	offset = narrow_arg[5];
	offset *= get_pagesize();
	return print_mmap(tcp, u_arg, offset);
}
#endif

/* Params are passed directly, offset is in bytes */
SYS_FUNC(mmap)
{
	unsigned long long offset = (unsigned long) tcp->u_arg[5];
#if defined(LINUX_MIPSN32) || defined(X32)
	/* Try test/x32_mmap.c */
	offset = tcp->ext_arg[5];
#endif
	/* Example of kernel-side handling of this variety of mmap:
	 * arch/x86/kernel/sys_x86_64.c::SYSCALL_DEFINE6(mmap, ...) calls
	 * sys_mmap_pgoff(..., off >> PAGE_SHIFT); i.e. off is in bytes,
	 * since the above code converts off to pages.
	 */
	return print_mmap(tcp, tcp->u_arg, offset);
}

/* Params are passed directly, offset is in pages */
SYS_FUNC(mmap_pgoff)
{
	/* Try test/mmap_offset_decode.c */
	unsigned long long offset;
	offset = (unsigned long) tcp->u_arg[5];
	offset *= get_pagesize();
	return print_mmap(tcp, tcp->u_arg, offset);
}

/* Params are passed directly, offset is in 4k units */
SYS_FUNC(mmap_4koff)
{
	unsigned long long offset;
	offset = (unsigned long) tcp->u_arg[5];
	offset <<= 12;
	return print_mmap(tcp, tcp->u_arg, offset);
}

SYS_FUNC(munmap)
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu",
			tcp->u_arg[0], tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(mprotect)
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, ",
			tcp->u_arg[0], tcp->u_arg[1]);
		printflags(mmap_prot, tcp->u_arg[2], "PROT_???");
	}
	return 0;
}

#include "xlat/mremap_flags.h"

SYS_FUNC(mremap)
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, %lu, ", tcp->u_arg[0], tcp->u_arg[1],
			tcp->u_arg[2]);
		printflags(mremap_flags, tcp->u_arg[3], "MREMAP_???");
#ifdef MREMAP_FIXED
		if ((tcp->u_arg[3] & (MREMAP_MAYMOVE | MREMAP_FIXED)) ==
		    (MREMAP_MAYMOVE | MREMAP_FIXED))
			tprintf(", %#lx", tcp->u_arg[4]);
#endif
	}
	return RVAL_HEX;
}

#include "xlat/madvise_cmds.h"

SYS_FUNC(madvise)
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
		printxval(madvise_cmds, tcp->u_arg[2], "MADV_???");
	}
	return 0;
}

#include "xlat/mlockall_flags.h"

SYS_FUNC(mlockall)
{
	if (entering(tcp)) {
		printflags(mlockall_flags, tcp->u_arg[0], "MCL_???");
	}
	return 0;
}

#include "xlat/mctl_sync.h"

SYS_FUNC(msync)
{
	if (entering(tcp)) {
		/* addr */
		tprintf("%#lx", tcp->u_arg[0]);
		/* len */
		tprintf(", %lu, ", tcp->u_arg[1]);
		/* flags */
		printflags(mctl_sync, tcp->u_arg[2], "MS_???");
	}
	return 0;
}

SYS_FUNC(mincore)
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
	} else {
		unsigned long i, len;
		char *vec = NULL;

		len = tcp->u_arg[1];
		if (syserror(tcp) || tcp->u_arg[2] == 0 ||
			(vec = malloc(len)) == NULL ||
			umoven(tcp, tcp->u_arg[2], len, vec) < 0)
			tprintf("%#lx", tcp->u_arg[2]);
		else {
			tprints("[");
			for (i = 0; i < len; i++) {
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

#if defined(ALPHA) || defined(IA64) || defined(SPARC) || defined(SPARC64)
SYS_FUNC(getpagesize)
{
	if (exiting(tcp))
		return RVAL_HEX;
	return 0;
}
#endif

SYS_FUNC(remap_file_pages)
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
		printflags(mmap_prot, tcp->u_arg[2], "PROT_???");
		tprintf(", %lu, ", tcp->u_arg[3]);
#ifdef MAP_TYPE
		printxval(mmap_flags, tcp->u_arg[4] & MAP_TYPE, "MAP_???");
		addflags(mmap_flags, tcp->u_arg[4] & ~MAP_TYPE);
#else
		printflags(mmap_flags, tcp->u_arg[4], "MAP_???");
#endif
	}
	return 0;
}

#define MPOL_DEFAULT    0
#define MPOL_PREFERRED  1
#define MPOL_BIND       2
#define MPOL_INTERLEAVE 3

#define MPOL_F_NODE     (1<<0)
#define MPOL_F_ADDR     (1<<1)

#define MPOL_MF_STRICT  (1<<0)
#define MPOL_MF_MOVE	(1<<1)
#define MPOL_MF_MOVE_ALL (1<<2)

#include "xlat/policies.h"
#include "xlat/mbindflags.h"
#include "xlat/mempolicyflags.h"
#include "xlat/move_pages_flags.h"

static void
get_nodes(struct tcb *tcp, unsigned long ptr, unsigned long maxnodes, int err)
{
	unsigned long nlongs, size, end;

	nlongs = (maxnodes + 8 * sizeof(long) - 1) / (8 * sizeof(long));
	size = nlongs * sizeof(long);
	end = ptr + size;
	if (nlongs == 0 || ((err || verbose(tcp)) && (size * 8 == maxnodes)
			    && (end > ptr))) {
		unsigned long n, cur, abbrev_end;
		int failed = 0;

		if (abbrev(tcp)) {
			abbrev_end = ptr + max_strlen * sizeof(long);
			if (abbrev_end < ptr)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints(", {");
		for (cur = ptr; cur < end; cur += sizeof(long)) {
			if (cur > ptr)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(n), &n) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%#0*lx", (int) sizeof(long) * 2 + 2, n);
		}
		tprints("}");
		if (failed)
			tprintf(" %#lx", ptr);
	} else
		tprintf(", %#lx", ptr);
	tprintf(", %lu", maxnodes);
}

SYS_FUNC(mbind)
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
		printxval(policies, tcp->u_arg[2], "MPOL_???");
		get_nodes(tcp, tcp->u_arg[3], tcp->u_arg[4], 0);
		tprints(", ");
		printflags(mbindflags, tcp->u_arg[5], "MPOL_???");
	}
	return 0;
}

SYS_FUNC(set_mempolicy)
{
	if (entering(tcp)) {
		printxval(policies, tcp->u_arg[0], "MPOL_???");
		get_nodes(tcp, tcp->u_arg[1], tcp->u_arg[2], 0);
	}
	return 0;
}

SYS_FUNC(get_mempolicy)
{
	if (exiting(tcp)) {
		int pol;
		if (tcp->u_arg[0] == 0)
			tprints("NULL");
		else if (syserror(tcp) || umove(tcp, tcp->u_arg[0], &pol) < 0)
			tprintf("%#lx", tcp->u_arg[0]);
		else
			printxval(policies, pol, "MPOL_???");
		get_nodes(tcp, tcp->u_arg[1], tcp->u_arg[2], syserror(tcp));
		tprintf(", %#lx, ", tcp->u_arg[3]);
		printflags(mempolicyflags, tcp->u_arg[4], "MPOL_???");
	}
	return 0;
}

SYS_FUNC(migrate_pages)
{
	if (entering(tcp)) {
		tprintf("%ld, ", (long) (pid_t) tcp->u_arg[0]);
		get_nodes(tcp, tcp->u_arg[2], tcp->u_arg[1], 0);
		tprints(", ");
		get_nodes(tcp, tcp->u_arg[3], tcp->u_arg[1], 0);
	}
	return 0;
}

SYS_FUNC(move_pages)
{
	if (entering(tcp)) {
		unsigned long npages = tcp->u_arg[1];
		tprintf("%ld, %lu, ", tcp->u_arg[0], npages);
		if (tcp->u_arg[2] == 0)
			tprints("NULL, ");
		else {
			unsigned int i;
			long puser = tcp->u_arg[2];
			tprints("{");
			for (i = 0; i < npages; ++i) {
				void *p;
				if (i > 0)
					tprints(", ");
				if (umove(tcp, puser, &p) < 0) {
					tprints("???");
					break;
				}
				tprintf("%p", p);
				puser += sizeof(void *);
			}
			tprints("}, ");
		}
		if (tcp->u_arg[3] == 0)
			tprints("NULL, ");
		else {
			unsigned int i;
			long nodeuser = tcp->u_arg[3];
			tprints("{");
			for (i = 0; i < npages; ++i) {
				int node;
				if (i > 0)
					tprints(", ");
				if (umove(tcp, nodeuser, &node) < 0) {
					tprints("???");
					break;
				}
				tprintf("%#x", node);
				nodeuser += sizeof(int);
			}
			tprints("}, ");
		}
	}
	if (exiting(tcp)) {
		unsigned long npages = tcp->u_arg[1];
		if (tcp->u_arg[4] == 0)
			tprints("NULL, ");
		else {
			unsigned int i;
			long statususer = tcp->u_arg[4];
			tprints("{");
			for (i = 0; i < npages; ++i) {
				int status;
				if (i > 0)
					tprints(", ");
				if (umove(tcp, statususer, &status) < 0) {
					tprints("???");
					break;
				}
				tprintf("%#x", status);
				statususer += sizeof(int);
			}
			tprints("}, ");
		}
		printflags(move_pages_flags, tcp->u_arg[5], "MPOL_???");
	}
	return 0;
}

#if defined(POWERPC)
SYS_FUNC(subpage_prot)
{
	if (entering(tcp)) {
		unsigned long cur, end, abbrev_end, entries;
		unsigned int entry;

		tprintf("%#lx, %#lx, ", tcp->u_arg[0], tcp->u_arg[1]);
		entries = tcp->u_arg[1] >> 16;
		if (!entries || !tcp->u_arg[2]) {
			tprints("{}");
			return 0;
		}
		cur = tcp->u_arg[2];
		end = cur + (sizeof(int) * entries);
		if (!verbose(tcp) || end < (unsigned long) tcp->u_arg[2]) {
			tprintf("%#lx", tcp->u_arg[2]);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = cur + (sizeof(int) * max_strlen);
			if (abbrev_end > end)
				abbrev_end = end;
		}
		else
			abbrev_end = end;
		tprints("{");
		for (; cur < end; cur += sizeof(int)) {
			if (cur > (unsigned long) tcp->u_arg[2])
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umove(tcp, cur, &entry) < 0) {
				tprintf("??? [%#lx]", cur);
				break;
			}
			else
				tprintf("%#08x", entry);
		}
		tprints("}");
	}

	return 0;
}
#endif
