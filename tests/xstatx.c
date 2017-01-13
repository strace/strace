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

# include <errno.h>
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

	if (p) {
		char buf[256];

		strftime(buf, sizeof(buf), "%FT%T%z", p);

		printf("%s", buf);
	} else {
		printf("%llu", zero_extend_signed_to_ull(t));
	}
}

# ifndef STRUCT_STAT
#  define STRUCT_STAT struct stat
#  define STRUCT_STAT_STR "struct stat"
#  define STRUCT_STAT_IS_STAT64 0
# endif
# ifndef SAMPLE_SIZE
#  define SAMPLE_SIZE ((libc_off_t) 43147718418ULL)
# endif

typedef off_t libc_off_t;

# define stat libc_stat
# define stat64 libc_stat64
# include <fcntl.h>
# include <sys/stat.h>
# undef stat
# undef stat64

# undef st_atime
# undef st_mtime
# undef st_ctime
# include "asm_stat.h"

# if STRUCT_STAT_IS_STAT64
#  undef HAVE_STRUCT_STAT_ST_MTIME_NSEC
#  if defined MPERS_IS_m32
#   ifdef HAVE_M32_STRUCT_STAT64_ST_MTIME_NSEC
#    define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#   endif
#  elif defined MPERS_IS_mx32
#   ifdef HAVE_MX32_STRUCT_STAT64_ST_MTIME_NSEC
#    define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#   endif
#  elif defined HAVE_STRUCT_STAT64_ST_MTIME_NSEC
#   define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#  endif /* MPERS_IS_m32 || MPERS_IS_mx32 || HAVE_STRUCT_STAT64_ST_MTIME_NSEC */
# else /* !STRUCT_STAT_IS_STAT64 */
#  if defined MPERS_IS_m32
#   undef HAVE_STRUCT_STAT_ST_MTIME_NSEC
#   ifdef HAVE_M32_STRUCT_STAT_ST_MTIME_NSEC
#    define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#   endif
#  elif defined MPERS_IS_mx32
#   undef HAVE_STRUCT_STAT_ST_MTIME_NSEC
#   ifdef HAVE_MX32_STRUCT_STAT_ST_MTIME_NSEC
#    define HAVE_STRUCT_STAT_ST_MTIME_NSEC 1
#   endif
#  endif /*  MPERS_IS_m32 || MPERS_IS_mx32 */
# endif /* STRUCT_STAT_IS_STAT64 */

# ifndef TEST_BOGUS_STRUCT_STAT
#  define TEST_BOGUS_STRUCT_STAT 1
# endif

# ifndef IS_FSTAT
#  define IS_FSTAT 0
# endif

# ifndef OLD_STAT
#  define OLD_STAT 0
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
# if OLD_STAT
	printf(", st_blksize=0, st_blocks=0");
# else /* !OLD_STAT */
	printf(", st_blksize=%llu", zero_extend_signed_to_ull(st->st_blksize));
	printf(", st_blocks=%llu", zero_extend_signed_to_ull(st->st_blocks));
# endif /* OLD_STAT */

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
# if defined(HAVE_STRUCT_STAT_ST_MTIME_NSEC) && !OLD_STAT
	if (st->st_atime_nsec)
		printf(".%09llu", zero_extend_signed_to_ull(st->st_atime_nsec));
# endif
	printf(", st_mtime=");
	print_time(sign_extend_unsigned_to_ll(st->st_mtime));
# if defined(HAVE_STRUCT_STAT_ST_MTIME_NSEC) && !OLD_STAT
	if (st->st_mtime_nsec)
		printf(".%09llu", zero_extend_signed_to_ull(st->st_mtime_nsec));
# endif
	printf(", st_ctime=");
	print_time(sign_extend_unsigned_to_ll(st->st_ctime));
# if defined(HAVE_STRUCT_STAT_ST_MTIME_NSEC) && !OLD_STAT
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
# if !IS_FSTAT
	static const char full[] = "/dev/full";
# endif
	static const char sample[] = TEST_SYSCALL_STR ".sample";
	STRUCT_STAT st[2];

	int rc;

	rc = create_sample(sample, SAMPLE_SIZE);
	if (rc) {
		(void) unlink(sample);
		return rc;
	}

# if TEST_BOGUS_STRUCT_STAT
	STRUCT_STAT *st_cut = tail_alloc(sizeof(long) * 4);
	rc = TEST_SYSCALL_INVOKE(sample, st_cut);
	PRINT_SYSCALL_HEADER(sample);
	printf("%p", st_cut);
	PRINT_SYSCALL_FOOTER(rc);
# endif

# if !IS_FSTAT
	rc = TEST_SYSCALL_INVOKE(full, st);
	PRINT_SYSCALL_HEADER(full);
	if (rc)
		printf("%p", st);
	else
		print_stat(st);
	PRINT_SYSCALL_FOOTER(rc);
# endif

	if ((rc = TEST_SYSCALL_INVOKE(sample, st))) {
# if OLD_STAT
		if (errno != EOVERFLOW)
# endif
		{
			perror(TEST_SYSCALL_STR);
			(void) unlink(sample);
			return 77;
		}
	}
	(void) unlink(sample);
	if (!rc && zero_extend_signed_to_ull(SAMPLE_SIZE) !=
	    zero_extend_signed_to_ull(st[0].st_size)) {
		fprintf(stderr, "Size mismatch: "
				"requested size(%llu) != st_size(%llu)\n",
			zero_extend_signed_to_ull(SAMPLE_SIZE),
			zero_extend_signed_to_ull(st[0].st_size));
		fprintf(stderr, "The most likely reason for this is incorrect"
				" definition of %s.\n"
				"Here is some diagnostics that might help:\n",
			STRUCT_STAT_STR);

#define LOG_STAT_OFFSETOF_SIZEOF(object, member)			\
		fprintf(stderr, "offsetof(%s, %s) = %zu"		\
				", sizeof(%s) = %zu\n",			\
				STRUCT_STAT_STR, #member,		\
				offsetof(STRUCT_STAT, member),		\
				#member, sizeof((object).member))

		LOG_STAT_OFFSETOF_SIZEOF(st[0], st_dev);
		LOG_STAT_OFFSETOF_SIZEOF(st[0], st_ino);
		LOG_STAT_OFFSETOF_SIZEOF(st[0], st_mode);
		LOG_STAT_OFFSETOF_SIZEOF(st[0], st_nlink);
		LOG_STAT_OFFSETOF_SIZEOF(st[0], st_uid);
		LOG_STAT_OFFSETOF_SIZEOF(st[0], st_gid);
		LOG_STAT_OFFSETOF_SIZEOF(st[0], st_rdev);
		LOG_STAT_OFFSETOF_SIZEOF(st[0], st_size);
# if !OLD_STAT
		LOG_STAT_OFFSETOF_SIZEOF(st[0], st_blksize);
		LOG_STAT_OFFSETOF_SIZEOF(st[0], st_blocks);
# endif /* !OLD_STAT */

		return 1;
	}

	PRINT_SYSCALL_HEADER(sample);
	if (rc)
		printf("%p", st);
	else
		print_stat(st);
	PRINT_SYSCALL_FOOTER(rc);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_FTRUNCATE && HAVE_FUTIMENS")

#endif
