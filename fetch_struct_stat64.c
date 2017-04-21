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

#include DEF_MPERS_TYPE(struct_stat64)

#include "asm_stat.h"

#if defined MPERS_IS_m32
# undef HAVE_STRUCT_STAT64
# undef HAVE_STRUCT_STAT64_ST_MTIME_NSEC
# ifdef HAVE_M32_STRUCT_STAT64
#  define HAVE_STRUCT_STAT64 1
#  ifdef HAVE_M32_STRUCT_STAT64_ST_MTIME_NSEC
#   define HAVE_STRUCT_STAT64_ST_MTIME_NSEC 1
#  endif /* HAVE_M32_STRUCT_STAT64_ST_MTIME_NSEC */
# endif /* HAVE_M32_STRUCT_STAT64 */
#elif defined MPERS_IS_mx32
# undef HAVE_STRUCT_STAT64
# undef HAVE_STRUCT_STAT64_ST_MTIME_NSEC
# ifdef HAVE_MX32_STRUCT_STAT64
#  define HAVE_STRUCT_STAT64 1
#  ifdef HAVE_MX32_STRUCT_STAT64_ST_MTIME_NSEC
#   define HAVE_STRUCT_STAT64_ST_MTIME_NSEC 1
#  endif /* HAVE_MX32_STRUCT_STAT64_ST_MTIME_NSEC */
# endif /* HAVE_MX32_STRUCT_STAT64 */
#endif /* MPERS_IS_m32 || MPERS_IS_mx32 */

#ifndef HAVE_STRUCT_STAT64
struct stat64 {};
#endif

typedef struct stat64 struct_stat64;

#include MPERS_DEFS

#include "stat.h"

#ifdef HAVE_STRUCT_STAT64_ST_MTIME_NSEC
# define TIME_NSEC(arg) zero_extend_signed_to_ull(arg)
# define HAVE_NSEC true
#else
# define TIME_NSEC(arg) 0
# define HAVE_NSEC false
#endif

MPERS_PRINTER_DECL(bool, fetch_struct_stat64,
		   struct tcb *const tcp, const kernel_ulong_t addr,
		   struct strace_stat *const dst)
{
#ifdef HAVE_STRUCT_STAT64
	struct_stat64 buf;
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
#else /* !HAVE_STRUCT_STAT64 */
	printaddr(addr);
	return false;
#endif
}
