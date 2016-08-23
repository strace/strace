/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
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
#include "asm_stat.h"

/* for S_IFMT */
#define stat libc_stat
#define stat64 libc_stat64
#include <sys/stat.h>
#undef stat
#undef stat64
/* These might be macros. */
#undef st_atime
#undef st_mtime
#undef st_ctime

#if defined MAJOR_IN_SYSMACROS
# include <sys/sysmacros.h>
#elif defined MAJOR_IN_MKDEV
# include <sys/mkdev.h>
#endif

/* several stats */

#include "printstat.h"

/* all locally defined structures provide these fields */
#undef HAVE_STRUCT_STAT_ST_MTIME_NSEC
#define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1

#undef STAT32_PERSONALITY
#if SUPPORTED_PERSONALITIES > 1
# include "stat32.h"
#endif

#ifdef STAT32_PERSONALITY
# define DO_PRINTSTAT do_printstat32
# define STRUCT_STAT struct stat32
# include "printstat.h"
#endif /* STAT32_PERSONALITY */

static void
printstat(struct tcb *tcp, long addr)
{
	struct stat statbuf;

#ifdef STAT32_PERSONALITY
	if (current_personality == STAT32_PERSONALITY) {
		struct stat32 statbuf;

		if (!umove_or_printaddr(tcp, addr, &statbuf))
			do_printstat32(tcp, &statbuf);
		return;
	}
#endif

	if (!umove_or_printaddr(tcp, addr, &statbuf))
		do_printstat(tcp, &statbuf);
}

SYS_FUNC(stat)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(fstat)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#if defined STAT32_PERSONALITY && !defined HAVE_STRUCT_STAT64
# if defined AARCH64 || defined X86_64 || defined X32
/*
 * Linux x86_64 and x32 have unified `struct stat' but their i386 personality
 * needs `struct stat64'.
 * linux/arch/x86/include/uapi/asm/stat.h defines `struct stat64' only for i386.
 *
 * Similarly, aarch64 has a unified `struct stat' but its arm personality
 * needs `struct stat64' (unlike x86, it shouldn't be packed).
 */
struct stat64 {
	unsigned long long	st_dev;
	unsigned char	__pad0[4];
	unsigned int	__st_ino;
	unsigned int	st_mode;
	unsigned int	st_nlink;
	unsigned int	st_uid;
	unsigned int	st_gid;
	unsigned long long	st_rdev;
	unsigned char	__pad3[4];
	long long	st_size;
	unsigned int	st_blksize;
	unsigned long long	st_blocks;
	unsigned int	st_atime;
	unsigned int	st_atime_nsec;
	unsigned int	st_mtime;
	unsigned int	st_mtime_nsec;
	unsigned int	st_ctime;
	unsigned int	st_ctime_nsec;
	unsigned long long	st_ino;
}
#  if defined X86_64 || defined X32
  ATTRIBUTE_PACKED
#   define STAT64_SIZE	96
#  else
#   define STAT64_SIZE	104
#  endif
;
#  define HAVE_STRUCT_STAT64	1
# else /* !(AARCH64 || X86_64 || X32) */
#  warning FIXME: check whether struct stat64 definition is needed for this architecture!
# endif
#endif /* STAT32_PERSONALITY && !HAVE_STRUCT_STAT64 */

#ifdef HAVE_STRUCT_STAT64

# define DO_PRINTSTAT do_printstat64
# define STRUCT_STAT struct stat64
# include "printstat.h"

static void
printstat64(struct tcb *tcp, long addr)
{
	struct stat64 statbuf;

# ifdef STAT64_SIZE
	(void) sizeof(char[sizeof statbuf == STAT64_SIZE ? 1 : -1]);
# endif

# if defined STAT32_PERSONALITY && !defined SPARC64
	if (current_personality != STAT32_PERSONALITY) {
		printstat(tcp, addr);
		return;
	}
# endif /* STAT32_PERSONALITY && !SPARC64 */

	if (!umove_or_printaddr(tcp, addr, &statbuf))
		do_printstat64(tcp, &statbuf);
}

SYS_FUNC(stat64)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstat64(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(fstat64)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstat64(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#else

SYS_FUNC(stat64)
{
	return sys_stat(tcp);
}

SYS_FUNC(fstat64)
{
	return sys_fstat(tcp);
}

#endif /* HAVE_STRUCT_STAT64 */

SYS_FUNC(newfstatat)
{
	if (entering(tcp)) {
		print_dirfd(tcp, tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
#if defined STAT32_PERSONALITY && !defined SPARC64
		if (current_personality == STAT32_PERSONALITY)
			printstat64(tcp, tcp->u_arg[2]);
		else
			printstat(tcp, tcp->u_arg[2]);
#elif defined HAVE_STRUCT_STAT64
		printstat64(tcp, tcp->u_arg[2]);
#else
		printstat(tcp, tcp->u_arg[2]);
#endif /* STAT32_PERSONALITY || HAVE_STRUCT_STAT64 */
		tprints(", ");
		printflags(at_flags, tcp->u_arg[3], "AT_???");
	}
	return 0;
}

#if defined(HAVE_STRUCT___OLD_KERNEL_STAT)

static void
convertoldstat(const struct __old_kernel_stat *oldbuf, struct stat *newbuf)
{
	memset(newbuf, 0, sizeof(*newbuf));
	newbuf->st_dev = oldbuf->st_dev;
	newbuf->st_ino = oldbuf->st_ino;
	newbuf->st_mode = oldbuf->st_mode;
	newbuf->st_nlink = oldbuf->st_nlink;
	newbuf->st_uid = oldbuf->st_uid;
	newbuf->st_gid = oldbuf->st_gid;
	newbuf->st_rdev = oldbuf->st_rdev;
	newbuf->st_size = oldbuf->st_size;
	newbuf->st_atime = oldbuf->st_atime;
	newbuf->st_mtime = oldbuf->st_mtime;
	newbuf->st_ctime = oldbuf->st_ctime;
}

static void
printoldstat(struct tcb *tcp, long addr)
{
	struct __old_kernel_stat statbuf;
	struct stat newstatbuf;

	if (!umove_or_printaddr(tcp, addr, &statbuf)) {
		convertoldstat(&statbuf, &newstatbuf);
		do_printstat(tcp, &newstatbuf);
	}
}

SYS_FUNC(oldstat)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printoldstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(oldfstat)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printoldstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#endif /* HAVE_STRUCT___OLD_KERNEL_STAT */
