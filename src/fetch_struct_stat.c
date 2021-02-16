/*
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_stat)

#include "asm_stat.h"

#if defined MPERS_IS_m32
# undef HAVE_STRUCT_STAT
# undef HAVE_STRUCT_STAT_ST_MTIME_NSEC
# ifdef HAVE_M32_STRUCT_STAT
#  define HAVE_STRUCT_STAT 1
#  ifdef HAVE_M32_STRUCT_STAT_ST_MTIME_NSEC
#   define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#  endif /* HAVE_M32_STRUCT_STAT_ST_MTIME_NSEC */
# endif /* HAVE_M32_STRUCT_STAT */
#elif defined MPERS_IS_mx32
# undef HAVE_STRUCT_STAT
# undef HAVE_STRUCT_STAT_ST_MTIME_NSEC
# ifdef HAVE_MX32_STRUCT_STAT
#  define HAVE_STRUCT_STAT 1
#  ifdef HAVE_MX32_STRUCT_STAT_ST_MTIME_NSEC
#   define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#  endif /* HAVE_MX32_STRUCT_STAT_ST_MTIME_NSEC */
# endif /* HAVE_MX32_STRUCT_STAT */
#else /* !MPERS_IS_m32 && !MPERS_IS_mx32 */
# define HAVE_STRUCT_STAT 1
#endif

#ifndef HAVE_STRUCT_STAT
struct stat {};
#endif

typedef struct stat struct_stat;

#include MPERS_DEFS

#include "stat.h"

#ifdef HAVE_STRUCT_STAT_ST_MTIME_NSEC
# define TIME_NSEC(arg) zero_extend_signed_to_ull(arg)
# define HAVE_NSEC true
#else
# define TIME_NSEC(arg) 0
# define HAVE_NSEC false
#endif

MPERS_PRINTER_DECL(bool, fetch_struct_stat,
		   struct tcb *const tcp, const kernel_ulong_t addr,
		   struct strace_stat *const dst)
{
#ifdef HAVE_STRUCT_STAT
	struct_stat buf;
	if (umove_or_printaddr(tcp, addr, &buf))
		return false;

	dst->dev = zero_extend_signed_to_ull(buf.st_dev);
	dst->ino = zero_extend_signed_to_ull(buf.st_ino);
	dst->rdev = zero_extend_signed_to_ull(buf.st_rdev);
	dst->size = zero_extend_signed_to_ull(buf.st_size);
	dst->blocks = zero_extend_signed_to_ull(buf.st_blocks);
	dst->blksize = zero_extend_signed_to_ull(buf.st_blksize);
	dst->mode = zero_extend_signed_to_ull(buf.st_mode);
	dst->nlink = zero_extend_signed_to_ull(buf.st_nlink);
	dst->uid = zero_extend_signed_to_ull(buf.st_uid);
	dst->gid = zero_extend_signed_to_ull(buf.st_gid);
	dst->atime = sign_extend_unsigned_to_ll(buf.st_atime);
	dst->ctime = sign_extend_unsigned_to_ll(buf.st_ctime);
	dst->mtime = sign_extend_unsigned_to_ll(buf.st_mtime);
	dst->atime_nsec = TIME_NSEC(buf.st_atime_nsec);
	dst->ctime_nsec = TIME_NSEC(buf.st_ctime_nsec);
	dst->mtime_nsec = TIME_NSEC(buf.st_mtime_nsec);
	dst->has_nsec = HAVE_NSEC;
	return true;
#else /* !HAVE_STRUCT_STAT */
	printaddr(addr);
	return false;
#endif
}
