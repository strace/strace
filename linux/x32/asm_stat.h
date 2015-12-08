/*
 * This is a replacement for x32 <asm/stat.h> which
 * appears to be wrong in older kernel headers.
 */

#ifndef STRACE_ASM_STAT_H

# define STRACE_ASM_STAT_H

# include "kernel_types.h"

struct stat {
	kernel_ulong_t	st_dev;
	kernel_ulong_t	st_ino;
	kernel_ulong_t	st_nlink;
	unsigned int	st_mode;
	unsigned int	st_uid;
	unsigned int	st_gid;
	unsigned int	pad0__;
	kernel_ulong_t	st_rdev;
	kernel_long_t	st_size;
	kernel_long_t	st_blksize;
	kernel_long_t	st_blocks;
	kernel_ulong_t	st_atime;
	kernel_ulong_t	st_atime_nsec;
	kernel_ulong_t	st_mtime;
	kernel_ulong_t	st_mtime_nsec;
	kernel_ulong_t	st_ctime;
	kernel_ulong_t	st_ctime_nsec;
	kernel_long_t	pad1__[3];
};

struct __old_kernel_stat {
	unsigned short st_dev;
	unsigned short st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned short st_rdev;
	unsigned int  st_size;
	unsigned int  st_atime;
	unsigned int  st_mtime;
	unsigned int  st_ctime;
};

#endif /* !STRACE_ASM_STAT_H */
