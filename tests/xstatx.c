/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

#if defined TEST_SYSCALL_NAME \
 && defined HAVE_FTRUNCATE && defined HAVE_FUTIMENS

# ifndef TEST_SYSCALL_INVOKE
#  error TEST_SYSCALL_INVOKE must be defined
# endif
# ifndef PRINT_SYSCALL_HEADER
#  error PRINT_SYSCALL_HEADER must be defined
# endif
# ifndef PRINT_SYSCALL_FOOTER
#  error PRINT_SYSCALL_FOOTER must be defined
# endif

# include <errno.h>
# include <stdio.h>
# include <stddef.h>
# include <time.h>
# include <unistd.h>

# if defined MAJOR_IN_SYSMACROS
#  include <sys/sysmacros.h>
# elif defined MAJOR_IN_MKDEV
#  include <sys/mkdev.h>
# else
#  include <sys/types.h>
# endif

static void
print_time(const time_t t)
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

typedef off_t libc_off_t;

# ifdef USE_ASM_STAT
#  define stat libc_stat
#  define stat64 libc_stat64
# endif
# include <fcntl.h>
# include <sys/stat.h>
# ifdef USE_ASM_STAT
#  undef stat
#  undef stat64
# endif

# ifdef USE_ASM_STAT
#  undef st_atime
#  undef st_mtime
#  undef st_ctime
#  undef dev_t
#  undef gid_t
#  undef ino_t
#  undef loff_t
#  undef mode_t
#  undef nlink_t
#  undef off64_t
#  undef off_t
#  undef time_t
#  undef uid_t
#  define dev_t __kernel_dev_t
#  define gid_t __kernel_gid_t
#  define ino_t __kernel_ino_t
#  define loff_t __kernel_loff_t
#  define mode_t __kernel_mode_t
#  define nlink_t __kernel_nlink_t
#  define off64_t __kernel_off64_t
#  define off_t __kernel_off_t
#  define time_t __kernel_time_t
#  define uid_t __kernel_uid_t
#  include "asm_stat.h"
# else
#  undef HAVE_STRUCT_STAT_ST_ATIME_NSEC
#  ifdef HAVE_STRUCT_STAT_ST_ATIM_TV_NSEC
#   define HAVE_STRUCT_STAT_ST_ATIME_NSEC 1
#   undef st_atime_nsec
#   define st_atime_nsec st_atim.tv_nsec
#  endif
#  undef HAVE_STRUCT_STAT_ST_MTIME_NSEC
#  ifdef HAVE_STRUCT_STAT_ST_MTIM_TV_NSEC
#   define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#   undef st_mtime_nsec
#   define st_mtime_nsec st_mtim.tv_nsec
#  endif
#  undef HAVE_STRUCT_STAT_ST_CTIME_NSEC
#  ifdef HAVE_STRUCT_STAT_ST_CTIM_TV_NSEC
#   define HAVE_STRUCT_STAT_ST_CTIME_NSEC 1
#   undef st_ctime_nsec
#   define st_ctime_nsec st_ctim.tv_nsec
#  endif
# endif

# ifndef STRUCT_STAT
#  define STRUCT_STAT struct stat
# endif
# ifndef SAMPLE_SIZE
#  define SAMPLE_SIZE 43147718418
# endif

static void
print_ftype(const unsigned int mode)
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
print_perms(const unsigned int mode)
{
	printf("%#o", mode & ~S_IFMT);
}

static void
print_stat(const STRUCT_STAT *st)
{
	printf("{st_dev=makedev(%u, %u)",
	       (unsigned int) major(st->st_dev),
	       (unsigned int) minor(st->st_dev));
	printf(", st_ino=%llu", (unsigned long long) st->st_ino);
	printf(", st_mode=");
	print_ftype(st->st_mode);
	printf("|");
	print_perms(st->st_mode);
	printf(", st_nlink=%u", (unsigned int) st->st_nlink);
	printf(", st_uid=%u", (unsigned int) st->st_uid);
	printf(", st_gid=%u", (unsigned int) st->st_gid);
	printf(", st_blksize=%u", (unsigned int) st->st_blksize);
	printf(", st_blocks=%u", (unsigned int) st->st_blocks);

	switch (st->st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
		printf(", st_rdev=makedev(%u, %u)",
		       (unsigned int) major(st->st_rdev),
		       (unsigned int) minor(st->st_rdev));
		break;
	default:
		printf(", st_size=%llu", (unsigned long long) st->st_size);
	}

	printf(", st_atime=");
	print_time(st->st_atime);
# ifdef HAVE_STRUCT_STAT_ST_ATIME_NSEC
	if (st->st_atime_nsec)
		printf(".%09lu", (unsigned long) st->st_atime_nsec);
# endif
	printf(", st_mtime=");
	print_time(st->st_mtime);
# ifdef HAVE_STRUCT_STAT_ST_MTIME_NSEC
	if (st->st_mtime_nsec)
		printf(".%09lu", (unsigned long) st->st_mtime_nsec);
# endif
	printf(", st_ctime=");
	print_time(st->st_ctime);
# ifdef HAVE_STRUCT_STAT_ST_CTIME_NSEC
	if (st->st_ctime_nsec)
		printf(".%09lu", (unsigned long) st->st_ctime_nsec);
# endif
	printf("}");
}

static int
create_sample(const char *fname, const libc_off_t size)
{
	static const struct timespec ts[] = {
		{-10843, 135}, {-10841, 246}
	};

	(void) close(0);
	if (open(fname, O_RDWR | O_CREAT | O_TRUNC, 0640)) {
		perror(fname);
		return 77;
	}
	if (ftruncate(0, size)) {
		perror("ftruncate");
		return 77;
	}
	if (futimens(0, ts)) {
		perror("futimens");
		return 77;
	}
	return 0;
}

# define stringify_(arg) #arg
# define stringify(arg) stringify_(arg)
# define TEST_SYSCALL_STR stringify(TEST_SYSCALL_NAME)
# define STRUCT_STAT_STR stringify(STRUCT_STAT)

int
main(void)
{
	static const char sample[] = TEST_SYSCALL_STR ".sample";
	STRUCT_STAT st[2];

	int rc = create_sample(sample, SAMPLE_SIZE);
	if (rc) {
		(void) unlink(sample);
		return rc;
	}

	if (TEST_SYSCALL_INVOKE(sample, st)) {
		perror(TEST_SYSCALL_STR);
		(void) unlink(sample);
		return 77;
	}
	(void) unlink(sample);
	if ((unsigned long long) SAMPLE_SIZE !=
	    (unsigned long long) st[0].st_size) {
		fprintf(stderr, "Size mismatch: "
				"requested size(%llu) != st_size(%llu)\n",
			(unsigned long long) SAMPLE_SIZE,
			(unsigned long long) st[0].st_size);
		fprintf(stderr, "The most likely reason for this is incorrect"
				" definition of %s.\n"
				"Here is some diagnostics that might help:\n",
			STRUCT_STAT_STR);
		fprintf(stderr, "offsetof(%s, st_dev) = %zu"
				", sizeof(st_dev) = %zu\n",
			STRUCT_STAT_STR, offsetof(STRUCT_STAT, st_dev),
			sizeof(st[0].st_dev));
		fprintf(stderr, "offsetof(%s, st_ino) = %zu"
				", sizeof(st_ino) = %zu\n",
			STRUCT_STAT_STR, offsetof(STRUCT_STAT, st_ino),
			sizeof(st[0].st_ino));
		fprintf(stderr, "offsetof(%s, st_mode) = %zu"
				", sizeof(st_mode) = %zu\n",
			STRUCT_STAT_STR, offsetof(STRUCT_STAT, st_mode),
			sizeof(st[0].st_mode));
		fprintf(stderr, "offsetof(%s, st_nlink) = %zu"
				", sizeof(st_nlink) = %zu\n",
			STRUCT_STAT_STR, offsetof(STRUCT_STAT, st_nlink),
			sizeof(st[0].st_nlink));
		fprintf(stderr, "offsetof(%s, st_uid) = %zu"
				", sizeof(st_uid) = %zu\n",
			STRUCT_STAT_STR, offsetof(STRUCT_STAT, st_uid),
			sizeof(st[0].st_uid));
		fprintf(stderr, "offsetof(%s, st_gid) = %zu"
				", sizeof(st_gid) = %zu\n",
			STRUCT_STAT_STR, offsetof(STRUCT_STAT, st_gid),
			sizeof(st[0].st_gid));
		fprintf(stderr, "offsetof(%s, st_rdev) = %zu"
				", sizeof(st_rdev) = %zu\n",
			STRUCT_STAT_STR, offsetof(STRUCT_STAT, st_rdev),
			sizeof(st[0].st_rdev));
		fprintf(stderr, "offsetof(%s, st_size) = %zu"
				", sizeof(st_size) = %zu\n",
			STRUCT_STAT_STR, offsetof(STRUCT_STAT, st_size),
			sizeof(st[0].st_size));
		fprintf(stderr, "offsetof(%s, st_blksize) = %zu"
				", sizeof(st_blksize) = %zu\n",
			STRUCT_STAT_STR, offsetof(STRUCT_STAT, st_blksize),
			sizeof(st[0].st_blksize));
		fprintf(stderr, "offsetof(%s, st_blocks) = %zu"
				", sizeof(st_blocks) = %zu\n",
			STRUCT_STAT_STR, offsetof(STRUCT_STAT, st_blocks),
			sizeof(st[0].st_blocks));
		return 77;
	}

	PRINT_SYSCALL_HEADER(sample);
	print_stat(st);
	PRINT_SYSCALL_FOOTER;

	puts("+++ exited with 0 +++");
	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
