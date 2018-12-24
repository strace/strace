/*
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_statfs)
#include DEF_MPERS_TYPE(struct_statfs64)

#include <linux/types.h>
#include <asm/statfs.h>
typedef struct statfs struct_statfs;
typedef struct statfs64 struct_statfs64;

#include MPERS_DEFS

#include "statfs.h"

MPERS_PRINTER_DECL(bool, fetch_struct_statfs,
		   struct tcb *const tcp, const kernel_ulong_t addr,
		   struct strace_statfs *const p)
{
	struct_statfs b;

	if (umove_or_printaddr(tcp, addr, &b))
		return false;

	p->f_type = zero_extend_signed_to_ull(b.f_type);
	p->f_bsize = zero_extend_signed_to_ull(b.f_bsize);
	p->f_blocks = zero_extend_signed_to_ull(b.f_blocks);
	p->f_bfree = zero_extend_signed_to_ull(b.f_bfree);
	p->f_bavail = zero_extend_signed_to_ull(b.f_bavail);
	p->f_files = zero_extend_signed_to_ull(b.f_files);
	p->f_ffree = zero_extend_signed_to_ull(b.f_ffree);
#if defined HAVE_STRUCT_STATFS_F_FSID_VAL
	p->f_fsid[0] = zero_extend_signed_to_ull(b.f_fsid.val[0]);
	p->f_fsid[1] = zero_extend_signed_to_ull(b.f_fsid.val[1]);
#elif defined HAVE_STRUCT_STATFS_F_FSID___VAL
	p->f_fsid[0] = zero_extend_signed_to_ull(b.f_fsid.__val[0]);
	p->f_fsid[1] = zero_extend_signed_to_ull(b.f_fsid.__val[1]);
#endif
	p->f_namelen = zero_extend_signed_to_ull(b.f_namelen);
#ifdef HAVE_STRUCT_STATFS_F_FRSIZE
	p->f_frsize = zero_extend_signed_to_ull(b.f_frsize);
#endif
#ifdef HAVE_STRUCT_STATFS_F_FLAGS
	p->f_flags = zero_extend_signed_to_ull(b.f_flags);
#endif

	return true;
}

#if defined ARM || (defined AARCH64 && defined IN_MPERS)
/* See arch/arm/kernel/sys_oabi-compat.c for details. */
# define COMPAT_STATFS64_PADDED_SIZE (sizeof(struct_statfs64) + 4)
#endif

MPERS_PRINTER_DECL(bool, fetch_struct_statfs64,
		   struct tcb *const tcp, const kernel_ulong_t addr,
		   const kernel_ulong_t size, struct strace_statfs *const p)
{
	struct_statfs64 b;

	if (sizeof(b) != size
#ifdef COMPAT_STATFS64_PADDED_SIZE
	    && sizeof(b) != COMPAT_STATFS64_PADDED_SIZE
#endif
	   ) {
		printaddr(addr);
		return false;
	}

	if (umove_or_printaddr(tcp, addr, &b))
		return false;

	p->f_type = zero_extend_signed_to_ull(b.f_type);
	p->f_bsize = zero_extend_signed_to_ull(b.f_bsize);
	p->f_blocks = zero_extend_signed_to_ull(b.f_blocks);
	p->f_bfree = zero_extend_signed_to_ull(b.f_bfree);
	p->f_bavail = zero_extend_signed_to_ull(b.f_bavail);
	p->f_files = zero_extend_signed_to_ull(b.f_files);
	p->f_ffree = zero_extend_signed_to_ull(b.f_ffree);
#if defined HAVE_STRUCT_STATFS64_F_FSID_VAL
	p->f_fsid[0] = zero_extend_signed_to_ull(b.f_fsid.val[0]);
	p->f_fsid[1] = zero_extend_signed_to_ull(b.f_fsid.val[1]);
#elif defined HAVE_STRUCT_STATFS64_F_FSID___VAL
	p->f_fsid[0] = zero_extend_signed_to_ull(b.f_fsid.__val[0]);
	p->f_fsid[1] = zero_extend_signed_to_ull(b.f_fsid.__val[1]);
#endif
	p->f_namelen = zero_extend_signed_to_ull(b.f_namelen);
#ifdef HAVE_STRUCT_STATFS64_F_FRSIZE
	p->f_frsize = zero_extend_signed_to_ull(b.f_frsize);
#endif
#ifdef HAVE_STRUCT_STATFS64_F_FLAGS
	p->f_flags = zero_extend_signed_to_ull(b.f_flags);
#endif

	return true;
}
