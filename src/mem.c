/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2000 PocketPenguins Inc.  Linux for Hitachi SuperH
 *                    port by Greg Banks <gbanks@pocketpenguins.com>
 * Copyright (c) 1999-2023 The strace developers.
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
		PRINT_VAL_X(flags);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_begin();

	tprint_flags_begin();

	printxvals_ex(flags & MAP_TYPE, "MAP_???", XLAT_STYLE_ABBREV,
		      mmap_flags, NULL);
	flags &= ~MAP_TYPE;

	const unsigned int mask = MAP_HUGE_MASK << MAP_HUGE_SHIFT;
	const unsigned int hugetlb_value = flags & mask;

	flags &= ~mask;
	if (flags) {
		tprint_flags_or();
		printflags_ex(flags, NULL, XLAT_STYLE_ABBREV,
			      mmap_flags, NULL);
	}

	if (hugetlb_value) {
		tprint_flags_or();
		tprint_shift_begin();
		PRINT_VAL_U(hugetlb_value >> MAP_HUGE_SHIFT);
		tprint_shift();
		/*
		 * print_xlat_u is not used here because the whole thing
		 * is potentially inside a comment already.
		 */
		tprints_string("MAP_HUGE_SHIFT");
		tprint_shift_end();
	}

	tprint_flags_end();

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
                tprint_comment_end();
}

static void
print_mmap(struct tcb *tcp, kernel_ulong_t *u_arg, unsigned long long offset)
{
	const kernel_ulong_t addr = u_arg[0];
	const kernel_ulong_t len = u_arg[1];
	const kernel_ulong_t prot = u_arg[2];
	const kernel_ulong_t flags = u_arg[3];
	const int fd = u_arg[4];

	/* addr */
	printaddr(addr);
	tprint_arg_next();

	/* length */
	PRINT_VAL_U(len);
	tprint_arg_next();

	/* prot */
	printflags64(mmap_prot, prot, "PROT_???");
	tprint_arg_next();

	/* flags */
	print_mmap_flags(flags);
	tprint_arg_next();

	/* fd */
	printfd(tcp, fd);
	tprint_arg_next();

	/* offset */
	PRINT_VAL_X(offset);
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
	/* addr */
	printaddr(tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	PRINT_VAL_U(tcp->u_arg[1]);

	return RVAL_DECODED;
}

static int
do_mprotect(struct tcb *tcp, bool has_pkey)
{
	/* addr */
	printaddr(tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	PRINT_VAL_U(tcp->u_arg[1]);
	tprint_arg_next();

	/* prot */
	printflags64(mmap_prot, tcp->u_arg[2], "PROT_???");

	if (has_pkey) {
		tprint_arg_next();
		/* pkey */
		PRINT_VAL_D((int) tcp->u_arg[3]);
	}

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
	/* old_address */
	printaddr(tcp->u_arg[0]);
	tprint_arg_next();

	/* old_size */
	PRINT_VAL_U(tcp->u_arg[1]);
	tprint_arg_next();

	/* new_size */
	PRINT_VAL_U(tcp->u_arg[2]);
	tprint_arg_next();

	/* flags */
	printflags64(mremap_flags, tcp->u_arg[3], "MREMAP_???");

	if ((tcp->u_arg[3] & (MREMAP_MAYMOVE | MREMAP_FIXED)) ==
	    (MREMAP_MAYMOVE | MREMAP_FIXED)) {
		/* new_address */
		tprint_arg_next();
		printaddr(tcp->u_arg[4]);
	}

	return RVAL_DECODED | RVAL_HEX;
}

#include "xlat/madvise_cmds.h"
#include "xlat/madvise_hppa_generic_cmds.h"

#if defined HPPA
# include "xlat/madvise_hppa_old_cmds.h"
#endif

SYS_FUNC(madvise)
{
	/* addr */
	printaddr(tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	PRINT_VAL_U(tcp->u_arg[1]);
	tprint_arg_next();

	/* advice */
	const unsigned int advice = tcp->u_arg[2];
#if defined HPPA
	/*
	 * hppa decided to be very special:  it used to have its own
	 * definitions for some MADV_* constants (just like Alpha,
	 * for example), but then (see Linux commit v6.2-rc1~39^2~7)
	 * decided to change their values, so their symbolic names
	 * are meaningless for the user now (which would also probably
	 * add spice to debugging old binaries with the newer kernels
	 * "in year 2025 (or later)"), and that forces us to state
	 * explicitly which variant of the constant value is used.
	 */
	const char *old_cmd = xlookup(madvise_hppa_old_cmds, advice);

	if (old_cmd) {
		PRINT_VAL_X(advice);
		if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW)
			tprintf_comment("old %s", old_cmd);
	} else {
		const char *new_cmd = xlookup(madvise_hppa_generic_cmds,
					      advice);

		if (new_cmd) {
			PRINT_VAL_X(advice);
			if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW)
				tprintf_comment("generic %s", new_cmd);
		} else {
			printxval(madvise_cmds, advice, "MADV_???");
		}
	}
#else
	printxvals(advice, "MADV_???",
		   madvise_cmds, madvise_hppa_generic_cmds, NULL);
#endif

	return RVAL_DECODED;
}

SYS_FUNC(process_madvise)
{
	const int pidfd = tcp->u_arg[0];
	const kernel_ulong_t addr = tcp->u_arg[1];
	const kernel_ulong_t len = tcp->u_arg[2];
	const unsigned int advice = tcp->u_arg[3];
	const unsigned int flags = tcp->u_arg[4];

	printfd(tcp, pidfd);
	tprint_arg_next();

	tprint_iov(tcp, len, addr, iov_decode_addr);
	tprint_arg_next();

	PRINT_VAL_U(len);
	tprint_arg_next();

	printxval(madvise_cmds, advice, "MADV_???");
	tprint_arg_next();

	PRINT_VAL_X(flags);

	return RVAL_DECODED;
}

SYS_FUNC(process_mrelease)
{
	const int pidfd = tcp->u_arg[0];
	const unsigned int flags = tcp->u_arg[1];

	printfd(tcp, pidfd);
	tprint_arg_next();

	PRINT_VAL_X(flags);

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
	tprint_arg_next();

	/* length */
	PRINT_VAL_U(tcp->u_arg[1]);
	tprint_arg_next();

	/* flags */
	printflags(mctl_sync, tcp->u_arg[2], "MS_???");

	return RVAL_DECODED;
}

#include "xlat/mlock_flags.h"

SYS_FUNC(mlock2)
{
	/* addr */
	printaddr(tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	PRINT_VAL_U(tcp->u_arg[1]);
	tprint_arg_next();

	/* flags */
	printflags(mlock_flags, tcp->u_arg[2], "MLOCK_???");

	return RVAL_DECODED;
}

static bool
print_mincore_entry(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	uint8_t val = (*(uint8_t *) elem_buf) & 1;
	PRINT_VAL_U(val);

	return true;
}

SYS_FUNC(mincore)
{
	if (entering(tcp)) {
		/* addr */
		printaddr(tcp->u_arg[0]);
		tprint_arg_next();

		/* length */
		PRINT_VAL_U(tcp->u_arg[1]);
		tprint_arg_next();
	} else {
		/* vec */
		const unsigned long page_size = get_pagesize();
		const unsigned long page_mask = page_size - 1;
		const kernel_ulong_t len = tcp->u_arg[1];
		const kernel_ulong_t nmemb =
			len / page_size + ((len & page_mask) ? 1 : 0);
		const kernel_ulong_t vec = tcp->u_arg[2];
		uint8_t entry;
		print_array(tcp, vec, nmemb, &entry, sizeof(entry),
			    tfetch_mem, print_mincore_entry, 0);
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

	/* addr */
	printaddr(addr);
	tprint_arg_next();

	/* size */
	PRINT_VAL_U(size);
	tprint_arg_next();

	/* prot */
	printflags64(mmap_prot, prot, "PROT_???");
	tprint_arg_next();

	/* pgoff */
	PRINT_VAL_U(pgoff);
	tprint_arg_next();

	/* flags */
	print_mmap_flags(flags);

	return RVAL_DECODED;
}

#if defined(POWERPC)
static bool
print_protmap_entry(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	PRINT_VAL_X(*(unsigned int *) elem_buf);

	return true;
}

SYS_FUNC(subpage_prot)
{
	kernel_ulong_t addr = tcp->u_arg[0];
	kernel_ulong_t len = tcp->u_arg[1];
	kernel_ulong_t nmemb = len >> 16;
	kernel_ulong_t map = tcp->u_arg[2];

	/* addr */
	printaddr(addr);
	tprint_arg_next();

	/* length */
	PRINT_VAL_U(len);
	tprint_arg_next();

	/* map */
	unsigned int entry;
	print_array(tcp, map, nmemb, &entry, sizeof(entry),
		    tfetch_mem, print_protmap_entry, 0);

	return RVAL_DECODED;
}
#endif
