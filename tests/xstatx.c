/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#if defined HAVE_FTRUNCATE && defined HAVE_FUTIMENS

# ifndef TEST_SYSCALL_STR
#  error TEST_SYSCALL_STR must be defined
# endif
# ifndef TEST_SYSCALL_INVOKE
#  error TEST_SYSCALL_INVOKE must be defined
# endif
# ifndef PRINT_SYSCALL_HEADER
#  error PRINT_SYSCALL_HEADER must be defined
# endif
# ifndef PRINT_SYSCALL_FOOTER
#  error PRINT_SYSCALL_FOOTER must be defined
# endif

# include <stdio.h>
# include <stddef.h>
# include <time.h>
# include <unistd.h>
# include <sys/sysmacros.h>

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
		printf("%llu", zero_extend_signed_to_ull(t));
}

# ifndef STRUCT_STAT
#  define STRUCT_STAT struct stat
#  define STRUCT_STAT_STR "struct stat"
#  define STRUCT_STAT_IS_STAT64 0
# endif
# ifndef SAMPLE_SIZE
#  define SAMPLE_SIZE 43147718418
# endif

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
#  include "asm_stat.h"

#  if STRUCT_STAT_IS_STAT64
#   undef HAVE_STRUCT_STAT_ST_MTIME_NSEC
#   if defined MPERS_IS_m32
#    ifdef HAVE_M32_STRUCT_STAT64_ST_MTIME_NSEC
#     define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#    endif
#   elif defined MPERS_IS_mx32
#    ifdef HAVE_MX32_STRUCT_STAT64_ST_MTIME_NSEC
#     define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#    endif
#   elif defined HAVE_STRUCT_STAT64_ST_MTIME_NSEC
#    define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#   endif /* MPERS_IS_m32 || MPERS_IS_mx32 || HAVE_STRUCT_STAT64_ST_MTIME_NSEC */
#  else /* !STRUCT_STAT_IS_STAT64 */
#   if defined MPERS_IS_m32
#    undef HAVE_STRUCT_STAT_ST_MTIME_NSEC
#    ifdef HAVE_M32_STRUCT_STAT_ST_MTIME_NSEC
#     define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#    endif
#   elif defined MPERS_IS_mx32
#    undef HAVE_STRUCT_STAT_ST_MTIME_NSEC
#    ifdef HAVE_MX32_STRUCT_STAT_ST_MTIME_NSEC
#     define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#    endif
#   endif /*  MPERS_IS_m32 || MPERS_IS_mx32 */
#  endif /* STRUCT_STAT_IS_STAT64 */

# else /* !USE_ASM_STAT */
#  undef HAVE_STRUCT_STAT_ST_MTIME_NSEC
#  ifdef HAVE_STRUCT_STAT_ST_MTIM_TV_NSEC
#   define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#   undef st_atime_nsec
#   define st_atime_nsec st_atim.tv_nsec
#   undef st_ctime_nsec
#   define st_ctime_nsec st_ctim.tv_nsec
#   undef st_mtime_nsec
#   define st_mtime_nsec st_mtim.tv_nsec
#  endif /* HAVE_STRUCT_STAT_ST_MTIM_TV_NSEC */
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
	       (unsigned int) major(zero_extend_signed_to_ull(st->st_dev)),
	       (unsigned int) minor(zero_extend_signed_to_ull(st->st_dev)));
	printf(", st_ino=%llu", zero_extend_signed_to_ull(st->st_ino));
	printf(", st_mode=");
	print_ftype(st->st_mode);
	printf("|");
	print_perms(st->st_mode);
	printf(", st_nlink=%llu", zero_extend_signed_to_ull(st->st_nlink));
	printf(", st_uid=%llu", zero_extend_signed_to_ull(st->st_uid));
	printf(", st_gid=%llu", zero_extend_signed_to_ull(st->st_gid));
	printf(", st_blksize=%llu", zero_extend_signed_to_ull(st->st_blksize));
	printf(", st_blocks=%llu", zero_extend_signed_to_ull(st->st_blocks));

	switch (st->st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
		printf(", st_rdev=makedev(%u, %u)",
		       (unsigned int) major(zero_extend_signed_to_ull(st->st_rdev)),
		       (unsigned int) minor(zero_extend_signed_to_ull(st->st_rdev)));
		break;
	default:
		printf(", st_size=%llu", zero_extend_signed_to_ull(st->st_size));
	}

	printf(", st_atime=");
	print_time(sign_extend_unsigned_to_ll(st->st_atime));
# ifdef HAVE_STRUCT_STAT_ST_MTIME_NSEC
	if (st->st_atime_nsec)
		printf(".%09llu", zero_extend_signed_to_ull(st->st_atime_nsec));
# endif
	printf(", st_mtime=");
	print_time(sign_extend_unsigned_to_ll(st->st_mtime));
# ifdef HAVE_STRUCT_STAT_ST_MTIME_NSEC
	if (st->st_mtime_nsec)
		printf(".%09llu", zero_extend_signed_to_ull(st->st_mtime_nsec));
# endif
	printf(", st_ctime=");
	print_time(sign_extend_unsigned_to_ll(st->st_ctime));
# ifdef HAVE_STRUCT_STAT_ST_MTIME_NSEC
	if (st->st_ctime_nsec)
		printf(".%09llu", zero_extend_signed_to_ull(st->st_ctime_nsec));
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
	if (zero_extend_signed_to_ull(SAMPLE_SIZE) !=
	    zero_extend_signed_to_ull(st[0].st_size)) {
		fprintf(stderr, "Size mismatch: "
				"requested size(%llu) != st_size(%llu)\n",
			zero_extend_signed_to_ull(SAMPLE_SIZE),
			zero_extend_signed_to_ull(st[0].st_size));
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

SKIP_MAIN_UNDEFINED("HAVE_FTRUNCATE && HAVE_FUTIMENS")

#endif
