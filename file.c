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

#undef dev_t
#undef ino_t
#undef mode_t
#undef nlink_t
#undef uid_t
#undef gid_t
#undef off_t
#undef loff_t
#define dev_t __kernel_dev_t
#define ino_t __kernel_ino_t
#define mode_t __kernel_mode_t
#define nlink_t __kernel_nlink_t
#define uid_t __kernel_uid_t
#define gid_t __kernel_gid_t
#define off_t __kernel_off_t
#define loff_t __kernel_loff_t

#include <asm/stat.h>

#undef dev_t
#undef ino_t
#undef mode_t
#undef nlink_t
#undef uid_t
#undef gid_t
#undef off_t
#undef loff_t
#define dev_t dev_t
#define ino_t ino_t
#define mode_t mode_t
#define nlink_t nlink_t
#define uid_t uid_t
#define gid_t gid_t
#define off_t off_t
#define loff_t loff_t

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

#undef STAT32_PERSONALITY
#if SUPPORTED_PERSONALITIES > 1
# if defined AARCH64 || defined X86_64 || defined X32
struct stat32 {
	unsigned int	st_dev;
	unsigned int	st_ino;
	unsigned short	st_mode;
	unsigned short	st_nlink;
	unsigned short	st_uid;
	unsigned short	st_gid;
	unsigned int	st_rdev;
	unsigned int	st_size;
	unsigned int	st_blksize;
	unsigned int	st_blocks;
	unsigned int	st_atime;
	unsigned int	st_atime_nsec;
	unsigned int	st_mtime;
	unsigned int	st_mtime_nsec;
	unsigned int	st_ctime;
	unsigned int	st_ctime_nsec;
	unsigned int	__unused4;
	unsigned int	__unused5;
};
#  ifdef AARCH64
#   define STAT32_PERSONALITY 0
#  else
#   define STAT32_PERSONALITY 1
#  endif
# elif defined POWERPC64
struct stat32 {
	unsigned int	st_dev;
	unsigned int	st_ino;
	unsigned int	st_mode;
	unsigned short	st_nlink;
	unsigned int	st_uid;
	unsigned int	st_gid;
	unsigned int	st_rdev;
	unsigned int	st_size;
	unsigned int	st_blksize;
	unsigned int	st_blocks;
	unsigned int	st_atime;
	unsigned int	st_atime_nsec;
	unsigned int	st_mtime;
	unsigned int	st_mtime_nsec;
	unsigned int	st_ctime;
	unsigned int	st_ctime_nsec;
	unsigned int	__unused4;
	unsigned int	__unused5;
};
#  define STAT32_PERSONALITY 1
# elif defined SPARC64
struct stat32 {
	unsigned short	st_dev;
	unsigned int	st_ino;
	unsigned short	st_mode;
	unsigned short	st_nlink;
	unsigned short	st_uid;
	unsigned short	st_gid;
	unsigned short	st_rdev;
	unsigned int	st_size;
	unsigned int	st_atime;
	unsigned int	st_atime_nsec;
	unsigned int	st_mtime;
	unsigned int	st_mtime_nsec;
	unsigned int	st_ctime;
	unsigned int	st_ctime_nsec;
	unsigned int	st_blksize;
	unsigned int	st_blocks;
	unsigned int	__unused4[2];
};
#  define STAT32_PERSONALITY 0
# elif defined SPARC
#  /* no 64-bit personalities */
# elif defined TILE
#  /* no 32-bit stat */
# else
#  warning FIXME: check whether struct stat32 definition is needed for this architecture!
# endif /* X86_64 || X32 || POWERPC64 */
#endif /* SUPPORTED_PERSONALITIES > 1 */

#ifdef STAT32_PERSONALITY
# define DO_PRINTSTAT do_printstat32
# define STRUCT_STAT struct stat32
# undef HAVE_STRUCT_STAT_ST_FLAGS
# undef HAVE_STRUCT_STAT_ST_FSTYPE
# undef HAVE_STRUCT_STAT_ST_GEN
# include "printstat.h"

static void
printstat32(struct tcb *tcp, long addr)
{
	struct stat32 statbuf;

	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}

	do_printstat32(tcp, &statbuf);
}
#endif /* STAT32_PERSONALITY */

#if defined(SPARC) || defined(SPARC64)

struct solstat {
	unsigned	st_dev;
	unsigned int	st_pad1[3];     /* network id */
	unsigned	st_ino;
	unsigned	st_mode;
	unsigned	st_nlink;
	unsigned	st_uid;
	unsigned	st_gid;
	unsigned	st_rdev;
	unsigned int	st_pad2[2];
	unsigned int	st_size;
	unsigned int	st_pad3;        /* st_size, off_t expansion */
	unsigned int	st_atime;
	unsigned int	st_atime_nsec;
	unsigned int	st_mtime;
	unsigned int	st_mtime_nsec;
	unsigned int	st_ctime;
	unsigned int	st_ctime_nsec;
	unsigned int	st_blksize;
	unsigned int	st_blocks;
	char		st_fstype[16];
	unsigned int	st_pad4[8];     /* expansion area */
};

# define DO_PRINTSTAT	do_printstat_sol
# define STRUCT_STAT	struct solstat
# define STAT_MAJOR(x)	(((x) >> 18) & 0x3fff)
# define STAT_MINOR(x)	((x) & 0x3ffff)
# undef HAVE_STRUCT_STAT_ST_FLAGS
# undef HAVE_STRUCT_STAT_ST_FSTYPE
# undef HAVE_STRUCT_STAT_ST_GEN
# include "printstat.h"

static void
printstatsol(struct tcb *tcp, long addr)
{
	struct solstat statbuf;

	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}

	do_printstat_sol(tcp, &statbuf);
}

#endif /* SPARC || SPARC64 */

static void
printstat(struct tcb *tcp, long addr)
{
	struct stat statbuf;

	if (!addr) {
		tprints("NULL");
		return;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}

#ifdef STAT32_PERSONALITY
	if (current_personality == STAT32_PERSONALITY) {
		printstat32(tcp, addr);
		return;
	}
#endif

#if defined(SPARC) || defined(SPARC64)
	if (current_personality == 1) {
		printstatsol(tcp, addr);
		return;
	}
#endif /* SPARC || SPARC64 */

	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}

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
# undef HAVE_STRUCT_STAT_ST_FLAGS
# undef HAVE_STRUCT_STAT_ST_FSTYPE
# undef HAVE_STRUCT_STAT_ST_GEN
# include "printstat.h"

static void
printstat64(struct tcb *tcp, long addr)
{
	struct stat64 statbuf;

# ifdef STAT64_SIZE
	(void) sizeof(char[sizeof statbuf == STAT64_SIZE ? 1 : -1]);
# endif

	if (!addr) {
		tprints("NULL");
		return;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}

# ifdef STAT32_PERSONALITY
	if (current_personality != STAT32_PERSONALITY) {
		printstat(tcp, addr);
		return;
	}
# endif /* STAT32_PERSONALITY */

	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}

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
#if defined STAT32_PERSONALITY
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

	if (!addr) {
		tprints("NULL");
		return;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}

# if defined(SPARC) || defined(SPARC64)
	if (current_personality == 1) {
		printstatsol(tcp, addr);
		return;
	}
# endif

	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}

	convertoldstat(&statbuf, &newstatbuf);
	do_printstat(tcp, &newstatbuf);
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

#if defined(SPARC) || defined(SPARC64)

SYS_FUNC(xstat)
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
		printstat(tcp, tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(fxstat)
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printfd(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
		printstat(tcp, tcp->u_arg[2]);
	}
	return 0;
}

#endif /* SPARC || SPARC64 */
