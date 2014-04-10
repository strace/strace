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

static unsigned long
get_pagesize()
{
	static unsigned long pagesize;

	if (!pagesize)
		pagesize = sysconf(_SC_PAGESIZE);
	return pagesize;
}

int
sys_brk(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%#lx", tcp->u_arg[0]);
	}
	return RVAL_HEX;
}

static const struct xlat mmap_prot[] = {
	XLAT(PROT_NONE),
	XLAT(PROT_READ),
	XLAT(PROT_WRITE),
	XLAT(PROT_EXEC),
#ifdef PROT_SEM
	XLAT(PROT_SEM),
#endif
#ifdef PROT_GROWSDOWN
	XLAT(PROT_GROWSDOWN),
#endif
#ifdef PROT_GROWSUP
	XLAT(PROT_GROWSUP),
#endif
#ifdef PROT_SAO
	XLAT(PROT_SAO),
#endif
	XLAT_END
};

static const struct xlat mmap_flags[] = {
	XLAT(MAP_SHARED),
	XLAT(MAP_PRIVATE),
	XLAT(MAP_FIXED),
#ifdef MAP_ANONYMOUS
	XLAT(MAP_ANONYMOUS),
#endif
#ifdef MAP_32BIT
	XLAT(MAP_32BIT),
#endif
#ifdef MAP_RENAME
	XLAT(MAP_RENAME),
#endif
#ifdef MAP_NORESERVE
	XLAT(MAP_NORESERVE),
#endif
#ifdef MAP_POPULATE
	XLAT(MAP_POPULATE),
#endif
#ifdef MAP_NONBLOCK
	XLAT(MAP_NONBLOCK),
#endif
	/*
	 * XXX - this was introduced in SunOS 4.x to distinguish between
	 * the old pre-4.x "mmap()", which:
	 *
	 *	only let you map devices with an "mmap" routine (e.g.,
	 *	frame buffers) in;
	 *
	 *	required you to specify the mapping address;
	 *
	 *	returned 0 on success and -1 on failure;
	 *
	 * memory and which, and the 4.x "mmap()" which:
	 *
	 *	can map plain files;
	 *
	 *	can be asked to pick where to map the file;
	 *
	 *	returns the address where it mapped the file on success
	 *	and -1 on failure.
	 *
	 * It's not actually used in source code that calls "mmap()"; the
	 * "mmap()" routine adds it for you.
	 *
	 * It'd be nice to come up with some way of eliminating it from
	 * the flags, e.g. reporting calls *without* it as "old_mmap()"
	 * and calls with it as "mmap()".
	 */
#ifdef _MAP_NEW
	XLAT(_MAP_NEW),
#endif
#ifdef MAP_GROWSDOWN
	XLAT(MAP_GROWSDOWN),
#endif
#ifdef MAP_DENYWRITE
	XLAT(MAP_DENYWRITE),
#endif
#ifdef MAP_EXECUTABLE
	XLAT(MAP_EXECUTABLE),
#endif
#ifdef MAP_INHERIT
	XLAT(MAP_INHERIT),
#endif
#ifdef MAP_FILE
	XLAT(MAP_FILE),
#endif
#ifdef MAP_LOCKED
	XLAT(MAP_LOCKED),
#endif
	/* FreeBSD ones */
#if defined(MAP_ANON) && (!defined(MAP_ANONYMOUS) || MAP_ANON != MAP_ANONYMOUS)
	XLAT(MAP_ANON),
#endif
#ifdef MAP_HASSEMAPHORE
	XLAT(MAP_HASSEMAPHORE),
#endif
#ifdef MAP_STACK
	XLAT(MAP_STACK),
#endif
#ifdef MAP_HUGETLB
	XLAT(MAP_HUGETLB),
#endif
#if defined MAP_UNINITIALIZED && MAP_UNINITIALIZED > 0
	XLAT(MAP_UNINITIALIZED),
#endif
#ifdef MAP_NOSYNC
	XLAT(MAP_NOSYNC),
#endif
#ifdef MAP_NOCORE
	XLAT(MAP_NOCORE),
#endif
	XLAT_END
};

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
int
sys_old_mmap(struct tcb *tcp)
{
	long u_arg[6];
#if defined(IA64)
	/*
	 * IA64 processes never call this routine, they only use the
	 * new 'sys_mmap' interface. Only IA32 processes come here.
	 */
	int i;
	unsigned narrow_arg[6];
	if (umoven(tcp, tcp->u_arg[0], sizeof(narrow_arg), (char *) narrow_arg) == -1)
		return 0;
	for (i = 0; i < 6; i++)
		u_arg[i] = (unsigned long) narrow_arg[i];
#elif defined(X86_64)
	/* We are here only in personality 1 (i386) */
	int i;
	unsigned narrow_arg[6];
	if (umoven(tcp, tcp->u_arg[0], sizeof(narrow_arg), (char *) narrow_arg) == -1)
		return 0;
	for (i = 0; i < 6; ++i)
		u_arg[i] = (unsigned long) narrow_arg[i];
#else
	if (umoven(tcp, tcp->u_arg[0], sizeof(u_arg), (char *) u_arg) == -1)
		return 0;
#endif
	return print_mmap(tcp, u_arg, (unsigned long) u_arg[5]);
}

#if defined(S390)
/* Params are pointed to by u_arg[0], offset is in pages */
int
sys_old_mmap_pgoff(struct tcb *tcp)
{
	long u_arg[5];
	int i;
	unsigned narrow_arg[6];
	unsigned long long offset;
	if (umoven(tcp, tcp->u_arg[0], sizeof(narrow_arg), (char *) narrow_arg) == -1)
		return 0;
	for (i = 0; i < 5; i++)
		u_arg[i] = (unsigned long) narrow_arg[i];
	offset = narrow_arg[5];
	offset *= get_pagesize();
	return print_mmap(tcp, u_arg, offset);
}
#endif

/* Params are passed directly, offset is in bytes */
int
sys_mmap(struct tcb *tcp)
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
int
sys_mmap_pgoff(struct tcb *tcp)
{
	/* Try test/mmap_offset_decode.c */
	unsigned long long offset;
	offset = (unsigned long) tcp->u_arg[5];
	offset *= get_pagesize();
	return print_mmap(tcp, tcp->u_arg, offset);
}

/* Params are passed directly, offset is in 4k units */
int
sys_mmap_4koff(struct tcb *tcp)
{
	unsigned long long offset;
	offset = (unsigned long) tcp->u_arg[5];
	offset <<= 12;
	return print_mmap(tcp, tcp->u_arg, offset);
}

int
sys_munmap(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu",
			tcp->u_arg[0], tcp->u_arg[1]);
	}
	return 0;
}

int
sys_mprotect(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, ",
			tcp->u_arg[0], tcp->u_arg[1]);
		printflags(mmap_prot, tcp->u_arg[2], "PROT_???");
	}
	return 0;
}

static const struct xlat mremap_flags[] = {
	XLAT(MREMAP_MAYMOVE),
#ifdef MREMAP_FIXED
	XLAT(MREMAP_FIXED),
#endif
	XLAT_END
};

int
sys_mremap(struct tcb *tcp)
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

static const struct xlat madvise_cmds[] = {
#ifdef MADV_NORMAL
	XLAT(MADV_NORMAL),
#endif
#ifdef MADV_RANDOM
	XLAT(MADV_RANDOM),
#endif
#ifdef MADV_SEQUENTIAL
	XLAT(MADV_SEQUENTIAL),
#endif
#ifdef MADV_WILLNEED
	XLAT(MADV_WILLNEED),
#endif
#ifdef MADV_DONTNEED
	XLAT(MADV_DONTNEED),
#endif
#ifdef MADV_REMOVE
	XLAT(MADV_REMOVE),
#endif
#ifdef MADV_DONTFORK
	XLAT(MADV_DONTFORK),
#endif
#ifdef MADV_DOFORK
	XLAT(MADV_DOFORK),
#endif
#ifdef MADV_HWPOISON
	XLAT(MADV_HWPOISON),
#endif
#ifdef MADV_SOFT_OFFLINE
	XLAT(MADV_SOFT_OFFLINE),
#endif
#ifdef MADV_MERGEABLE
	XLAT(MADV_MERGEABLE),
#endif
#ifdef MADV_UNMERGEABLE
	XLAT(MADV_UNMERGEABLE),
#endif
#ifdef MADV_HUGEPAGE
	XLAT(MADV_HUGEPAGE),
#endif
#ifdef MADV_NOHUGEPAGE
	XLAT(MADV_NOHUGEPAGE),
#endif
#ifdef MADV_DONTDUMP
	XLAT(MADV_DONTDUMP),
#endif
#ifdef MADV_DODUMP
	XLAT(MADV_DODUMP),
#endif
	XLAT_END
};

int
sys_madvise(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
		printxval(madvise_cmds, tcp->u_arg[2], "MADV_???");
	}
	return 0;
}

static const struct xlat mlockall_flags[] = {
#ifdef MCL_CURRENT
	XLAT(MCL_CURRENT),
#endif
#ifdef MCL_FUTURE
	XLAT(MCL_FUTURE),
#endif
	XLAT_END
};

int
sys_mlockall(struct tcb *tcp)
{
	if (entering(tcp)) {
		printflags(mlockall_flags, tcp->u_arg[0], "MCL_???");
	}
	return 0;
}

#ifdef MS_ASYNC

static const struct xlat mctl_sync[] = {
#ifdef MS_SYNC
	XLAT(MS_SYNC),
#endif
	XLAT(MS_ASYNC),
	XLAT(MS_INVALIDATE),
	XLAT_END
};

int
sys_msync(struct tcb *tcp)
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

#endif /* MS_ASYNC */

#ifdef MC_SYNC

static const struct xlat mctl_funcs[] = {
	XLAT(MC_LOCK),
	XLAT(MC_LOCKAS),
	XLAT(MC_SYNC),
	XLAT(MC_UNLOCK),
	XLAT(MC_UNLOCKAS),
	XLAT_END
};

static const struct xlat mctl_lockas[] = {
	XLAT(MCL_CURRENT),
	XLAT(MCL_FUTURE),
	XLAT_END
};

int
sys_mctl(struct tcb *tcp)
{
	int arg, function;

	if (entering(tcp)) {
		/* addr */
		tprintf("%#lx", tcp->u_arg[0]);
		/* len */
		tprintf(", %lu, ", tcp->u_arg[1]);
		/* function */
		function = tcp->u_arg[2];
		printflags(mctl_funcs, function, "MC_???");
		/* arg */
		arg = tcp->u_arg[3];
		tprints(", ");
		switch (function) {
		case MC_SYNC:
			printflags(mctl_sync, arg, "MS_???");
			break;
		case MC_LOCKAS:
			printflags(mctl_lockas, arg, "MCL_???");
			break;
		default:
			tprintf("%#x", arg);
			break;
		}
	}
	return 0;
}

#endif /* MC_SYNC */

int
sys_mincore(struct tcb *tcp)
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
int
sys_getpagesize(struct tcb *tcp)
{
	if (exiting(tcp))
		return RVAL_HEX;
	return 0;
}
#endif

int
sys_remap_file_pages(struct tcb *tcp)
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

static const struct xlat policies[] = {
	XLAT(MPOL_DEFAULT),
	XLAT(MPOL_PREFERRED),
	XLAT(MPOL_BIND),
	XLAT(MPOL_INTERLEAVE),
	XLAT_END
};

static const struct xlat mbindflags[] = {
	XLAT(MPOL_MF_STRICT),
	XLAT(MPOL_MF_MOVE),
	XLAT(MPOL_MF_MOVE_ALL),
	XLAT_END
};

static const struct xlat mempolicyflags[] = {
	XLAT(MPOL_F_NODE),
	XLAT(MPOL_F_ADDR),
	XLAT_END
};

static const struct xlat move_pages_flags[] = {
	XLAT(MPOL_MF_MOVE),
	XLAT(MPOL_MF_MOVE_ALL),
	XLAT_END
};

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
			if (umoven(tcp, cur, sizeof(n), (char *) &n) < 0) {
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

int
sys_mbind(struct tcb *tcp)
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

int
sys_set_mempolicy(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(policies, tcp->u_arg[0], "MPOL_???");
		get_nodes(tcp, tcp->u_arg[1], tcp->u_arg[2], 0);
	}
	return 0;
}

int
sys_get_mempolicy(struct tcb *tcp)
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

int
sys_migrate_pages(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, ", (long) (pid_t) tcp->u_arg[0]);
		get_nodes(tcp, tcp->u_arg[2], tcp->u_arg[1], 0);
		tprints(", ");
		get_nodes(tcp, tcp->u_arg[3], tcp->u_arg[1], 0);
	}
	return 0;
}

int
sys_move_pages(struct tcb *tcp)
{
	if (entering(tcp)) {
		unsigned long npages = tcp->u_arg[1];
		tprintf("%ld, %lu, ", tcp->u_arg[0], npages);
		if (tcp->u_arg[2] == 0)
			tprints("NULL, ");
		else {
			int i;
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
			int i;
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
			int i;
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
int
sys_subpage_prot(struct tcb *tcp)
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
		if (!verbose(tcp) || end < tcp->u_arg[2]) {
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
			if (cur > tcp->u_arg[2])
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
