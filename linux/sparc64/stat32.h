#ifndef STRACE_STAT32_H
#define STRACE_STAT32_H

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

# define STAT32_PERSONALITY 1

#endif /* !STRACE_STAT32_H */
