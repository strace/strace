/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2000 PocketPenguins Inc.  Linux for Hitachi SuperH
 *                    port by Greg Banks <gbanks@pocketpenguins.com>
 * Copyright (c) 1999-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/mman.h>
#include <sys/mman.h>

unsigned long
get_pagesize(void)
{
	static unsigned long pagesize;

	if (!pagesize) {
		long ret = sysconf(_SC_PAGESIZE);

		if (ret < 0)
			perror_func_msg_and_die("sysconf(_SC_PAGESIZE)");
		if (ret == 0)
			error_func_msg_and_die("sysconf(_SC_PAGESIZE) "
					       "returned 0");

		pagesize = (unsigned long) ret;
	}

	return pagesize;
}

SYS_FUNC(brk)
{
	printaddr(tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_HEX;
}

#include "xlat/mmap_prot.h"
#include "xlat/mmap_flags.h"

#ifndef MAP_HUGE_SHIFT
# define MAP_HUGE_SHIFT 26
#endif

#ifndef MAP_HUGE_MASK
# define MAP_HUGE_MASK 0x3f
#endif

static void
print_mmap_flags(kernel_ulong_t flags)
{
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		tprintf("%#" PRI_klx, flags);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprints(" /* ");

	printxvals_ex(flags & MAP_TYPE, "MAP_???", XLAT_STYLE_ABBREV,
		      mmap_flags, NULL);
	flags &= ~MAP_TYPE;

	const unsigned int mask = MAP_HUGE_MASK << MAP_HUGE_SHIFT;
	const unsigned int hugetlb_value = flags & mask;

	flags &= ~mask;
	if (flags) {
		tprints("|");
		printflags_ex(flags, NULL, XLAT_STYLE_ABBREV,
			      mmap_flags, NULL);
	}

	if (hugetlb_value)
		tprintf("|%u<<MAP_HUGE_SHIFT",
			hugetlb_value >> MAP_HUGE_SHIFT);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
                tprints(" */");
}

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
	print_mmap_flags(flags);
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

#if HAVE_ARCH_OLD_MMAP
/* Params are pointed to by u_arg[0], offset is in bytes */
SYS_FUNC(old_mmap)
{
	kernel_ulong_t *args =
		fetch_indirect_syscall_args(tcp, tcp->u_arg[0], 6);

	if (args)
		print_mmap(tcp, args, args[5]);
	else
		printaddr(tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_HEX;
}

# if HAVE_ARCH_OLD_MMAP_PGOFF
/* Params are pointed to by u_arg[0], offset is in pages */
SYS_FUNC(old_mmap_pgoff)
{
	kernel_ulong_t *args =
		fetch_indirect_syscall_args(tcp, tcp->u_arg[0], 6);

	if (args) {
		unsigned long long offset;

		offset = args[5];
		offset *= get_pagesize();

		print_mmap(tcp, args, offset);
	} else {
		printaddr(tcp->u_arg[0]);
	}

	return RVAL_DECODED | RVAL_HEX;
}
# endif /* HAVE_ARCH_OLD_MMAP_PGOFF */
#endif /* HAVE_ARCH_OLD_MMAP */

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

SYS_FUNC(process_madvise)
{
	const int pidfd = tcp->u_arg[0];
	const kernel_ulong_t addr = tcp->u_arg[1];
	const kernel_ulong_t len = tcp->u_arg[2];
	const unsigned int behavior = tcp->u_arg[3];
	const unsigned int flags = tcp->u_arg[4];

	printfd(tcp, pidfd);

	tprints(", ");
	tprint_iov(tcp, len, addr, IOV_DECODE_ADDR);

	tprintf(", %" PRI_klu ", ", len);

	printxval(madvise_cmds, behavior, "MADV_???");

	tprintf(", %#x", flags);

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
	print_mmap_flags(flags);

	return RVAL_DECODED;
}

#if defined(POWERPC)
static bool
print_protmap_entry(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	tprintf("%#08x", *(unsigned int *) elem_buf);

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
		    tfetch_mem, print_protmap_entry, 0);

	return RVAL_DECODED;
}
#endif
