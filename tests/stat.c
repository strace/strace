#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#if defined MAJOR_IN_SYSMACROS
# include <sys/sysmacros.h>
#elif defined MAJOR_IN_MKDEV
# include <sys/mkdev.h>
#else
# include <sys/types.h>
#endif

#undef STAT_PREFIX
#undef NR_stat

#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS == 64
# include <sys/stat.h>
# define STAT_PREFIX "(stat(64)?\\(|newfstatat\\(AT_FDCWD, )"
#else
# include <sys/syscall.h>
# if defined __NR_stat
#  define NR_stat __NR_stat
#  define STAT_PREFIX "stat\\("
# elif defined __NR_newstat
#  define NR_stat __NR_newstat
#  define STAT_PREFIX "newstat\\("
# endif
# ifdef STAT_PREFIX
/* for S_IFMT */
#  define stat libc_stat
#  define stat64 libc_stat64
#  include <sys/stat.h>
#  undef stat
#  undef stat64
#  undef st_atime
#  undef st_mtime
#  undef st_ctime

#  undef dev_t
#  undef ino_t
#  undef mode_t
#  undef nlink_t
#  undef uid_t
#  undef gid_t
#  undef off_t
#  undef loff_t
#  define dev_t __kernel_dev_t
#  define ino_t __kernel_ino_t
#  define mode_t __kernel_mode_t
#  define nlink_t __kernel_nlink_t
#  define uid_t __kernel_uid_t
#  define gid_t __kernel_gid_t
#  define off_t __kernel_off_t
#  define loff_t __kernel_loff_t
#  include <asm/stat.h>
#  endif /* STAT_PREFIX */
#endif /* _FILE_OFFSET_BITS */

#ifdef STAT_PREFIX

static void
print_ftype(unsigned int mode)
{
	if (S_ISREG(mode))
		printf("S_IFREG");
	else if (S_ISDIR(mode))
		printf("S_IFDIR");
	else if (S_ISCHR(mode))
		printf("S_IFCHR");
	else if (S_ISBLK(mode))
		printf("S_IFBLK");
	else
		printf("%#o", mode & S_IFMT);
}

static void
print_perms(unsigned int mode)
{
	printf("%#o", mode & ~S_IFMT);
}

static void
print_time(time_t t)
{
	if (!t) {
		printf("0");
		return;
	}

	struct tm *p = localtime(&t);

	if (p)
		printf("%02d/%02d/%02d-%02d:%02d:%02d",
		       p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
		       p->tm_hour, p->tm_min, p->tm_sec);
	else
		printf("%llu", (unsigned long long) t);
}

int
main(int ac, const char **av)
{
	assert(ac == 2);
	struct stat stb;

#ifdef NR_stat
	if (sizeof(stb.st_size) > 4)
		return 77;
	assert(syscall(NR_stat, av[1], &stb) == 0);
#else
	assert(stat(av[1], &stb) == 0);
#endif

	printf(STAT_PREFIX "\"%s\", \\{", av[1]);
	printf("st_dev=makedev\\(%u, %u\\)",
	       (unsigned int) major(stb.st_dev),
	       (unsigned int) minor(stb.st_dev));
	printf(", st_ino=%llu", (unsigned long long) stb.st_ino);
	printf(", st_mode=");
		print_ftype(stb.st_mode);
		printf("\\|");
		print_perms(stb.st_mode);
	printf(", st_nlink=%u", (unsigned int) stb.st_nlink);
	printf(", st_uid=%u", (unsigned int) stb.st_uid);
	printf(", st_gid=%u", (unsigned int) stb.st_gid);
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	printf(", st_blksize=%u", (unsigned int) stb.st_blksize);
#endif
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
	printf(", st_blocks=%u", (unsigned int) stb.st_blocks);
#endif

	switch (stb.st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
#ifdef HAVE_STRUCT_STAT_ST_RDEV
		printf(", st_rdev=makedev\\(%u, %u\\)",
		       (unsigned int) major(stb.st_rdev),
		       (unsigned int) minor(stb.st_rdev));
#else
		printf(", st_size=makedev\\(%u, %u\\)",
		       (unsigned int) major(stb.st_size),
		       (unsigned int) minor(stb.st_size));
#endif
		break;
	default:
		printf(", st_size=%llu", (unsigned long long) stb.st_size);
	}

	printf(", st_atime=");
		print_time(stb.st_atime);
	printf(", st_mtime=");
		print_time(stb.st_mtime);
	printf(", st_ctime=");
		print_time(stb.st_ctime);
	printf("(, st_flags=[0-9]+)?");
	printf("(, st_fstype=[^,]*)?");
	printf("(, st_gen=[0-9]+)?");
	printf("\\}");
#ifndef NR_stat
	printf("(, 0)?");
#endif
	printf("\\) += 0\n");
	return 0;
}

#else /* !STAT_PREFIX */
int main(void)
{
	return 77;
}
#endif
