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

#if defined(SPARC) || defined(SPARC64)
struct stat {
	unsigned short	st_dev;
	unsigned int	st_ino;
	unsigned short	st_mode;
	short		st_nlink;
	unsigned short	st_uid;
	unsigned short	st_gid;
	unsigned short	st_rdev;
	unsigned int	st_size;
	int		st_atime;
	unsigned int	__unused1;
	int		st_mtime;
	unsigned int	__unused2;
	int		st_ctime;
	unsigned int	__unused3;
	int		st_blksize;
	int		st_blocks;
	unsigned int	__unused4[2];
};
# if defined(SPARC64)
struct stat_sparc64 {
	unsigned int	st_dev;
	unsigned long	st_ino;
	unsigned int	st_mode;
	unsigned int	st_nlink;
	unsigned int	st_uid;
	unsigned int	st_gid;
	unsigned int	st_rdev;
	long		st_size;
	long		st_atime;
	long		st_mtime;
	long		st_ctime;
	long		st_blksize;
	long		st_blocks;
	unsigned long	__unused4[2];
};
# endif /* SPARC64 */
# define stat kernel_stat
# include <asm/stat.h>
# undef stat
#elif defined(X32)
struct stat {
	unsigned long long	st_dev;
	unsigned long long	st_ino;
	unsigned long long	st_nlink;

	unsigned int		st_mode;
	unsigned int		st_uid;
	unsigned int		st_gid;
	unsigned int		__pad0;
	unsigned long long	st_rdev;
	long long		st_size;
	long long		st_blksize;
	long long		st_blocks;

	unsigned long long	st_atime;
	unsigned long long	st_atime_nsec;
	unsigned long long	st_mtime;
	unsigned long long	st_mtime_nsec;
	unsigned long long	st_ctime;
	unsigned long long	st_ctime_nsec;
	long long		__unused[3];
};

struct stat64 {
	unsigned long long	st_dev;
	unsigned char		__pad0[4];
	unsigned long		__st_ino;
	unsigned int		st_mode;
	unsigned int		st_nlink;
	unsigned long		st_uid;
	unsigned long		st_gid;
	unsigned long long	st_rdev;
	unsigned char		__pad3[4];
	long long		st_size;
	unsigned long		st_blksize;
	unsigned long long	st_blocks;
	unsigned long		st_atime;
	unsigned long		st_atime_nsec;
	unsigned long		st_mtime;
	unsigned int		st_mtime_nsec;
	unsigned long		st_ctime;
	unsigned long		st_ctime_nsec;
	unsigned long long	st_ino;
} __attribute__((packed));
# define HAVE_STAT64	1

struct __old_kernel_stat {
	unsigned short st_dev;
	unsigned short st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned short st_rdev;
	unsigned int   st_size;
	unsigned int   st_atime;
	unsigned int   st_mtime;
	unsigned int   st_ctime;
};
#else
# undef dev_t
# undef ino_t
# undef mode_t
# undef nlink_t
# undef uid_t
# undef gid_t
# undef off_t
# undef loff_t
# define dev_t __kernel_dev_t
# define ino_t __kernel_ino_t
# define mode_t __kernel_mode_t
# define nlink_t __kernel_nlink_t
# define uid_t __kernel_uid_t
# define gid_t __kernel_gid_t
# define off_t __kernel_off_t
# define loff_t __kernel_loff_t

# include <asm/stat.h>

# undef dev_t
# undef ino_t
# undef mode_t
# undef nlink_t
# undef uid_t
# undef gid_t
# undef off_t
# undef loff_t
# define dev_t dev_t
# define ino_t ino_t
# define mode_t mode_t
# define nlink_t nlink_t
# define uid_t uid_t
# define gid_t gid_t
# define off_t off_t
# define loff_t loff_t
#endif

#define stat libc_stat
#define stat64 libc_stat64
#include <sys/stat.h>
#undef stat
#undef stat64
/* These might be macros. */
#undef st_atime
#undef st_mtime
#undef st_ctime

#ifdef MAJOR_IN_SYSMACROS
# include <sys/sysmacros.h>
#endif

#ifdef MAJOR_IN_MKDEV
# include <sys/mkdev.h>
#endif

/* several stats */

#if defined(SPARC) || defined(SPARC64)
typedef struct {
	int     tv_sec;
	int     tv_nsec;
} timestruct_t;

struct solstat {
	unsigned        st_dev;
	int             st_pad1[3];     /* network id */
	unsigned        st_ino;
	unsigned        st_mode;
	unsigned        st_nlink;
	unsigned        st_uid;
	unsigned        st_gid;
	unsigned        st_rdev;
	int             st_pad2[2];
	int             st_size;
	int             st_pad3;        /* st_size, off_t expansion */
	timestruct_t    st_atime;
	timestruct_t    st_mtime;
	timestruct_t    st_ctime;
	int             st_blksize;
	int             st_blocks;
	char            st_fstype[16];
	int             st_pad4[8];     /* expansion area */
};

static void
printstatsol(struct tcb *tcp, long addr)
{
	struct solstat statbuf;

	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}
	if (!abbrev(tcp)) {
		tprintf("{st_dev=makedev(%lu, %lu), st_ino=%lu, st_mode=%s, ",
			(unsigned long) ((statbuf.st_dev >> 18) & 0x3fff),
			(unsigned long) (statbuf.st_dev & 0x3ffff),
			(unsigned long) statbuf.st_ino,
			sprintmode(statbuf.st_mode));
		tprintf("st_nlink=%lu, st_uid=%lu, st_gid=%lu, ",
			(unsigned long) statbuf.st_nlink,
			(unsigned long) statbuf.st_uid,
			(unsigned long) statbuf.st_gid);
		tprintf("st_blksize=%lu, ", (unsigned long) statbuf.st_blksize);
		tprintf("st_blocks=%lu, ", (unsigned long) statbuf.st_blocks);
	}
	else
		tprintf("{st_mode=%s, ", sprintmode(statbuf.st_mode));
	switch (statbuf.st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
		tprintf("st_rdev=makedev(%lu, %lu), ",
			(unsigned long) ((statbuf.st_rdev >> 18) & 0x3fff),
			(unsigned long) (statbuf.st_rdev & 0x3ffff));
		break;
	default:
		tprintf("st_size=%u, ", statbuf.st_size);
		break;
	}
	if (!abbrev(tcp)) {
		tprintf("st_atime=%s, ", sprinttime(statbuf.st_atime.tv_sec));
		tprintf("st_mtime=%s, ", sprinttime(statbuf.st_mtime.tv_sec));
		tprintf("st_ctime=%s}", sprinttime(statbuf.st_ctime.tv_sec));
	}
	else
		tprints("...}");
}

# if defined(SPARC64)
static void
printstat_sparc64(struct tcb *tcp, long addr)
{
	struct stat_sparc64 statbuf;

	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}

	if (!abbrev(tcp)) {
		tprintf("{st_dev=makedev(%lu, %lu), st_ino=%lu, st_mode=%s, ",
			(unsigned long) major(statbuf.st_dev),
			(unsigned long) minor(statbuf.st_dev),
			(unsigned long) statbuf.st_ino,
			sprintmode(statbuf.st_mode));
		tprintf("st_nlink=%lu, st_uid=%lu, st_gid=%lu, ",
			(unsigned long) statbuf.st_nlink,
			(unsigned long) statbuf.st_uid,
			(unsigned long) statbuf.st_gid);
		tprintf("st_blksize=%lu, ",
			(unsigned long) statbuf.st_blksize);
		tprintf("st_blocks=%lu, ",
			(unsigned long) statbuf.st_blocks);
	}
	else
		tprintf("{st_mode=%s, ", sprintmode(statbuf.st_mode));
	switch (statbuf.st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
		tprintf("st_rdev=makedev(%lu, %lu), ",
			(unsigned long) major(statbuf.st_rdev),
			(unsigned long) minor(statbuf.st_rdev));
		break;
	default:
		tprintf("st_size=%lu, ", statbuf.st_size);
		break;
	}
	if (!abbrev(tcp)) {
		tprintf("st_atime=%s, ", sprinttime(statbuf.st_atime));
		tprintf("st_mtime=%s, ", sprinttime(statbuf.st_mtime));
		tprintf("st_ctime=%s}", sprinttime(statbuf.st_ctime));
	}
	else
		tprints("...}");
}
# endif /* SPARC64 */
#endif /* SPARC[64] */

#if defined POWERPC64
struct stat_powerpc32 {
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

static void
printstat_powerpc32(struct tcb *tcp, long addr)
{
	struct stat_powerpc32 statbuf;

	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}

	if (!abbrev(tcp)) {
		tprintf("{st_dev=makedev(%u, %u), st_ino=%u, st_mode=%s, ",
			major(statbuf.st_dev), minor(statbuf.st_dev),
			statbuf.st_ino,
			sprintmode(statbuf.st_mode));
		tprintf("st_nlink=%u, st_uid=%u, st_gid=%u, ",
			statbuf.st_nlink, statbuf.st_uid, statbuf.st_gid);
		tprintf("st_blksize=%u, ", statbuf.st_blksize);
		tprintf("st_blocks=%u, ", statbuf.st_blocks);
	}
	else
		tprintf("{st_mode=%s, ", sprintmode(statbuf.st_mode));
	switch (statbuf.st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
		tprintf("st_rdev=makedev(%lu, %lu), ",
			(unsigned long) major(statbuf.st_rdev),
			(unsigned long) minor(statbuf.st_rdev));
		break;
	default:
		tprintf("st_size=%u, ", statbuf.st_size);
		break;
	}
	if (!abbrev(tcp)) {
		tprintf("st_atime=%s, ", sprinttime(statbuf.st_atime));
		tprintf("st_mtime=%s, ", sprinttime(statbuf.st_mtime));
		tprintf("st_ctime=%s}", sprinttime(statbuf.st_ctime));
	}
	else
		tprints("...}");
}
#endif /* POWERPC64 */

#include "xlat/fileflags.h"

static void
realprintstat(struct tcb *tcp, struct stat *statbuf)
{
	if (!abbrev(tcp)) {
		tprintf("{st_dev=makedev(%lu, %lu), st_ino=%lu, st_mode=%s, ",
			(unsigned long) major(statbuf->st_dev),
			(unsigned long) minor(statbuf->st_dev),
			(unsigned long) statbuf->st_ino,
			sprintmode(statbuf->st_mode));
		tprintf("st_nlink=%lu, st_uid=%lu, st_gid=%lu, ",
			(unsigned long) statbuf->st_nlink,
			(unsigned long) statbuf->st_uid,
			(unsigned long) statbuf->st_gid);
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
		tprintf("st_blksize=%lu, ", (unsigned long) statbuf->st_blksize);
#endif
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
		tprintf("st_blocks=%lu, ", (unsigned long) statbuf->st_blocks);
#endif
	}
	else
		tprintf("{st_mode=%s, ", sprintmode(statbuf->st_mode));
	switch (statbuf->st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
#ifdef HAVE_STRUCT_STAT_ST_RDEV
		tprintf("st_rdev=makedev(%lu, %lu), ",
			(unsigned long) major(statbuf->st_rdev),
			(unsigned long) minor(statbuf->st_rdev));
#else /* !HAVE_STRUCT_STAT_ST_RDEV */
		tprintf("st_size=makedev(%lu, %lu), ",
			(unsigned long) major(statbuf->st_size),
			(unsigned long) minor(statbuf->st_size));
#endif /* !HAVE_STRUCT_STAT_ST_RDEV */
		break;
	default:
		tprintf("st_size=%lu, ", (unsigned long) statbuf->st_size);
		break;
	}
	if (!abbrev(tcp)) {
		tprintf("st_atime=%s, ", sprinttime(statbuf->st_atime));
		tprintf("st_mtime=%s, ", sprinttime(statbuf->st_mtime));
		tprintf("st_ctime=%s", sprinttime(statbuf->st_ctime));
#if HAVE_STRUCT_STAT_ST_FLAGS
		tprints(", st_flags=");
		printflags(fileflags, statbuf->st_flags, "UF_???");
#endif
#if HAVE_STRUCT_STAT_ST_ACLCNT
		tprintf(", st_aclcnt=%d", statbuf->st_aclcnt);
#endif
#if HAVE_STRUCT_STAT_ST_LEVEL
		tprintf(", st_level=%ld", statbuf->st_level);
#endif
#if HAVE_STRUCT_STAT_ST_FSTYPE
		tprintf(", st_fstype=%.*s",
			(int) sizeof statbuf->st_fstype, statbuf->st_fstype);
#endif
#if HAVE_STRUCT_STAT_ST_GEN
		tprintf(", st_gen=%u", statbuf->st_gen);
#endif
		tprints("}");
	}
	else
		tprints("...}");
}

#ifndef X32
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

#if defined(SPARC) || defined(SPARC64)
	if (current_personality == 1) {
		printstatsol(tcp, addr);
		return;
	}
#ifdef SPARC64
	else if (current_personality == 2) {
		printstat_sparc64(tcp, addr);
		return;
	}
#endif
#endif /* SPARC[64] */

#if defined POWERPC64
	if (current_personality == 1) {
		printstat_powerpc32(tcp, addr);
		return;
	}
#endif

	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}

	realprintstat(tcp, &statbuf);
}
#else /* X32 */
# define printstat printstat64
#endif

#if !defined HAVE_STAT64 && (defined AARCH64 || defined X86_64)
/*
 * Linux x86_64 has unified `struct stat' but its i386 biarch needs
 * `struct stat64'.  Its <asm-i386/stat.h> definition expects 32-bit `long'.
 * <linux/include/asm-x86_64/ia32.h> is not in the public includes set.
 * __GNUC__ is needed for the required __attribute__ below.
 *
 * Similarly, aarch64 has a unified `struct stat' but its arm personality
 * needs `struct stat64' (which also expects a 32-bit `long' but which
 * shouldn't be packed).
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
# if defined X86_64
   __attribute__((packed))
#  define STAT64_SIZE	96
#else
#  define STAT64_SIZE	104
# endif
;
# define HAVE_STAT64	1
#endif

#ifdef HAVE_STAT64
static void
printstat64(struct tcb *tcp, long addr)
{
#ifdef X32
	struct stat statbuf;
#else
	struct stat64 statbuf;
#endif

#ifdef STAT64_SIZE
	(void) sizeof(char[sizeof statbuf == STAT64_SIZE ? 1 : -1]);
#endif

	if (!addr) {
		tprints("NULL");
		return;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}

#if defined(SPARC) || defined(SPARC64)
	if (current_personality == 1) {
		printstatsol(tcp, addr);
		return;
	}
# ifdef SPARC64
	else if (current_personality == 2) {
		printstat_sparc64(tcp, addr);
		return;
	}
# endif
#endif /* SPARC[64] */

#if defined AARCH64
	if (current_personality != 0) {
		printstat(tcp, addr);
		return;
	}
#endif
#if defined X86_64
	if (current_personality != 1) {
		printstat(tcp, addr);
		return;
	}
#endif

	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}

	if (!abbrev(tcp)) {
		tprintf("{st_dev=makedev(%lu, %lu), st_ino=%llu, st_mode=%s, ",
			(unsigned long) major(statbuf.st_dev),
			(unsigned long) minor(statbuf.st_dev),
			(unsigned long long) statbuf.st_ino,
			sprintmode(statbuf.st_mode));
		tprintf("st_nlink=%lu, st_uid=%lu, st_gid=%lu, ",
			(unsigned long) statbuf.st_nlink,
			(unsigned long) statbuf.st_uid,
			(unsigned long) statbuf.st_gid);
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
		tprintf("st_blksize=%lu, ",
			(unsigned long) statbuf.st_blksize);
#endif /* HAVE_STRUCT_STAT_ST_BLKSIZE */
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
		tprintf("st_blocks=%lu, ", (unsigned long) statbuf.st_blocks);
#endif /* HAVE_STRUCT_STAT_ST_BLOCKS */
	}
	else
		tprintf("{st_mode=%s, ", sprintmode(statbuf.st_mode));
	switch (statbuf.st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
#ifdef HAVE_STRUCT_STAT_ST_RDEV
		tprintf("st_rdev=makedev(%lu, %lu), ",
			(unsigned long) major(statbuf.st_rdev),
			(unsigned long) minor(statbuf.st_rdev));
#else /* !HAVE_STRUCT_STAT_ST_RDEV */
		tprintf("st_size=makedev(%lu, %lu), ",
			(unsigned long) major(statbuf.st_size),
			(unsigned long) minor(statbuf.st_size));
#endif /* !HAVE_STRUCT_STAT_ST_RDEV */
		break;
	default:
		tprintf("st_size=%llu, ", (unsigned long long) statbuf.st_size);
		break;
	}
	if (!abbrev(tcp)) {
		tprintf("st_atime=%s, ", sprinttime(statbuf.st_atime));
		tprintf("st_mtime=%s, ", sprinttime(statbuf.st_mtime));
		tprintf("st_ctime=%s", sprinttime(statbuf.st_ctime));
#if HAVE_STRUCT_STAT_ST_FLAGS
		tprints(", st_flags=");
		printflags(fileflags, statbuf.st_flags, "UF_???");
#endif
#if HAVE_STRUCT_STAT_ST_ACLCNT
		tprintf(", st_aclcnt=%d", statbuf.st_aclcnt);
#endif
#if HAVE_STRUCT_STAT_ST_LEVEL
		tprintf(", st_level=%ld", statbuf.st_level);
#endif
#if HAVE_STRUCT_STAT_ST_FSTYPE
		tprintf(", st_fstype=%.*s",
			(int) sizeof statbuf.st_fstype, statbuf.st_fstype);
#endif
#if HAVE_STRUCT_STAT_ST_GEN
		tprintf(", st_gen=%u", statbuf.st_gen);
#endif
		tprints("}");
	}
	else
		tprints("...}");
}
#endif /* HAVE_STAT64 */

#if defined(HAVE_STRUCT___OLD_KERNEL_STAT)
static void
convertoldstat(const struct __old_kernel_stat *oldbuf, struct stat *newbuf)
{
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
	newbuf->st_blksize = 0; /* not supported in old_stat */
	newbuf->st_blocks = 0; /* not supported in old_stat */
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
	realprintstat(tcp, &newstatbuf);
}
#endif

int
sys_stat(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef X32
static void
printstat64_x32(struct tcb *tcp, long addr)
{
	struct stat64 statbuf;

	if (!addr) {
		tprints("NULL");
		return;
	}
	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}

	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}

	if (!abbrev(tcp)) {
		tprintf("{st_dev=makedev(%lu, %lu), st_ino=%llu, st_mode=%s, ",
			(unsigned long) major(statbuf.st_dev),
			(unsigned long) minor(statbuf.st_dev),
			(unsigned long long) statbuf.st_ino,
			sprintmode(statbuf.st_mode));
		tprintf("st_nlink=%lu, st_uid=%lu, st_gid=%lu, ",
			(unsigned long) statbuf.st_nlink,
			(unsigned long) statbuf.st_uid,
			(unsigned long) statbuf.st_gid);
		tprintf("st_blksize=%lu, ",
			(unsigned long) statbuf.st_blksize);
		tprintf("st_blocks=%lu, ", (unsigned long) statbuf.st_blocks);
	}
	else
		tprintf("{st_mode=%s, ", sprintmode(statbuf.st_mode));
	switch (statbuf.st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
		tprintf("st_rdev=makedev(%lu, %lu), ",
			(unsigned long) major(statbuf.st_rdev),
			(unsigned long) minor(statbuf.st_rdev));
		break;
	default:
		tprintf("st_size=%llu, ", (unsigned long long) statbuf.st_size);
		break;
	}
	if (!abbrev(tcp)) {
		tprintf("st_atime=%s, ", sprinttime(statbuf.st_atime));
		tprintf("st_mtime=%s, ", sprinttime(statbuf.st_mtime));
		tprintf("st_ctime=%s", sprinttime(statbuf.st_ctime));
		tprints("}");
	}
	else
		tprints("...}");
}
#endif /* X32 */

int
sys_stat64(struct tcb *tcp)
{
#ifdef HAVE_STAT64
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
# ifdef X32
		printstat64_x32(tcp, tcp->u_arg[1]);
# else
		printstat64(tcp, tcp->u_arg[1]);
# endif
	}
	return 0;
#else
	return printargs(tcp);
#endif
}

int
sys_newfstatat(struct tcb *tcp)
{
	if (entering(tcp)) {
		print_dirfd(tcp, tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
#ifdef POWERPC64
		if (current_personality == 0)
			printstat(tcp, tcp->u_arg[2]);
		else
			printstat64(tcp, tcp->u_arg[2]);
#elif defined HAVE_STAT64
		printstat64(tcp, tcp->u_arg[2]);
#else
		printstat(tcp, tcp->u_arg[2]);
#endif
		tprints(", ");
		printflags(at_flags, tcp->u_arg[3], "AT_???");
	}
	return 0;
}

#if defined(HAVE_STRUCT___OLD_KERNEL_STAT)
int
sys_oldstat(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printoldstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}
#endif

int
sys_fstat(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_fstat64(struct tcb *tcp)
{
#ifdef HAVE_STAT64
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
# ifdef X32
		printstat64_x32(tcp, tcp->u_arg[1]);
# else
		printstat64(tcp, tcp->u_arg[1]);
# endif
	}
	return 0;
#else
	return printargs(tcp);
#endif
}

#if defined(HAVE_STRUCT___OLD_KERNEL_STAT)
int
sys_oldfstat(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printoldstat(tcp, tcp->u_arg[1]);
	}
	return 0;
}
#endif

#if defined(SPARC) || defined(SPARC64)

int
sys_xstat(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
# ifdef _STAT64_VER
		if (tcp->u_arg[0] == _STAT64_VER)
			printstat64(tcp, tcp->u_arg[2]);
		else
# endif
		printstat(tcp, tcp->u_arg[2]);
	}
	return 0;
}

int
sys_fxstat(struct tcb *tcp)
{
	if (entering(tcp))
		tprintf("%ld, %ld, ", tcp->u_arg[0], tcp->u_arg[1]);
	else {
# ifdef _STAT64_VER
		if (tcp->u_arg[0] == _STAT64_VER)
			printstat64(tcp, tcp->u_arg[2]);
		else
# endif
		printstat(tcp, tcp->u_arg[2]);
	}
	return 0;
}

#endif /* SPARC || SPARC64 */
