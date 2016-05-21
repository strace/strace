/*
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include DEF_MPERS_TYPE(struct_statfs)
#include DEF_MPERS_TYPE(struct_statfs64)

#include <linux/types.h>
#include <asm/statfs.h>
typedef struct statfs struct_statfs;
typedef struct statfs64 struct_statfs64;

#include MPERS_DEFS

#include "statfs.h"

#define ASSIGN_NUMBER(dst, src)				\
	if (sizeof(src) == sizeof(int))			\
		dst = (unsigned int) (src);		\
	else if (sizeof(src) == sizeof(long))		\
		dst = (unsigned long) (src);		\
	else						\
		dst = (unsigned long long) (src)

MPERS_PRINTER_DECL(bool, fetch_struct_statfs,
		   struct tcb *tcp, const long addr, struct strace_statfs *p)
{
	struct_statfs b;

	if (umove_or_printaddr(tcp, addr, &b))
		return false;

	ASSIGN_NUMBER(p->f_type, b.f_type);
	ASSIGN_NUMBER(p->f_bsize, b.f_bsize);
	ASSIGN_NUMBER(p->f_blocks, b.f_blocks);
	ASSIGN_NUMBER(p->f_bfree, b.f_bfree);
	ASSIGN_NUMBER(p->f_bavail, b.f_bavail);
	ASSIGN_NUMBER(p->f_files, b.f_files);
	ASSIGN_NUMBER(p->f_ffree, b.f_ffree);
#if defined HAVE_STRUCT_STATFS_F_FSID_VAL
	ASSIGN_NUMBER(p->f_fsid[0], b.f_fsid.val[0]);
	ASSIGN_NUMBER(p->f_fsid[1], b.f_fsid.val[1]);
#elif defined HAVE_STRUCT_STATFS_F_FSID___VAL
	ASSIGN_NUMBER(p->f_fsid[0], b.f_fsid.__val[0]);
	ASSIGN_NUMBER(p->f_fsid[1], b.f_fsid.__val[1]);
#endif
	ASSIGN_NUMBER(p->f_namelen, b.f_namelen);
#ifdef HAVE_STRUCT_STATFS_F_FRSIZE
	ASSIGN_NUMBER(p->f_frsize, b.f_frsize);
#endif
#ifdef HAVE_STRUCT_STATFS_F_FLAGS
	ASSIGN_NUMBER(p->f_flags, b.f_flags);
#endif

	return true;
}

#if defined ARM || (defined AARCH64 && defined IN_MPERS)
/* See arch/arm/kernel/sys_oabi-compat.c for details. */
# define COMPAT_STATFS64_PADDED_SIZE (sizeof(struct_statfs64) + 4)
#endif

MPERS_PRINTER_DECL(bool, fetch_struct_statfs64,
		   struct tcb *tcp, const long addr, const unsigned long size,
		   struct strace_statfs *p)
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

	ASSIGN_NUMBER(p->f_type, b.f_type);
	ASSIGN_NUMBER(p->f_bsize, b.f_bsize);
	ASSIGN_NUMBER(p->f_blocks, b.f_blocks);
	ASSIGN_NUMBER(p->f_bfree, b.f_bfree);
	ASSIGN_NUMBER(p->f_bavail, b.f_bavail);
	ASSIGN_NUMBER(p->f_files, b.f_files);
	ASSIGN_NUMBER(p->f_ffree, b.f_ffree);
#if defined HAVE_STRUCT_STATFS64_F_FSID_VAL
	ASSIGN_NUMBER(p->f_fsid[0], b.f_fsid.val[0]);
	ASSIGN_NUMBER(p->f_fsid[1], b.f_fsid.val[1]);
#elif defined HAVE_STRUCT_STATFS64_F_FSID___VAL
	ASSIGN_NUMBER(p->f_fsid[0], b.f_fsid.__val[0]);
	ASSIGN_NUMBER(p->f_fsid[1], b.f_fsid.__val[1]);
#endif
	ASSIGN_NUMBER(p->f_namelen, b.f_namelen);
#ifdef HAVE_STRUCT_STATFS64_F_FRSIZE
	ASSIGN_NUMBER(p->f_frsize, b.f_frsize);
#endif
#ifdef HAVE_STRUCT_STATFS64_F_FLAGS
	ASSIGN_NUMBER(p->f_flags, b.f_flags);
#endif

	return true;
}
