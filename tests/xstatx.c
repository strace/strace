/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

# include "print_fields.h"
# include "statx.h"

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
# define statx libc_statx
# define statx_timestamp libc_statx_timestamp
struct statx;
# include <fcntl.h>
# include <sys/stat.h>
# undef statx_timestamp
# undef statx
# undef stat64
# undef stat

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

# ifndef IS_STATX
#  define IS_STATX 0
# endif

# if !XLAT_RAW /* Fixes -Wunused warning */
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
# endif

static void
print_st_mode(const unsigned int mode)
{
# if XLAT_RAW
	printf("%#o", mode);
# elif XLAT_VERBOSE
	printf("%#o /* ", mode);
	print_ftype(mode);
	printf("|");
	print_perms(mode);
	printf(" */");
# else
	print_ftype(mode);
	printf("|");
	print_perms(mode);
# endif
}

# if !IS_STATX

static const char *
sprint_makedev(const unsigned long long val)
{
	static char devid[256];
	int ret;

#  if XLAT_RAW
	ret = snprintf(devid, sizeof(devid),
			"%#llx", val);
#  elif XLAT_VERBOSE
	ret = snprintf(devid, sizeof(devid),
			"%#llx /* makedev(%#x, %#x) */",
			val, major(val), minor(val));
#  else /* XLAT_ABBREV */
	ret = snprintf(devid, sizeof(devid),
			"makedev(%#x, %#x)",
			major(val), minor(val));
#  endif
	if (ret < 0)
		perror_msg_and_fail("sprint_makedev(%llx)", val);
	if ((unsigned) ret >= sizeof(devid))
		error_msg_and_fail("sprint_makedev(%llx): buffer "
					   "overflow", val);
	return devid;
}


static void
print_stat(const STRUCT_STAT *st)
{
	unsigned long long dev, rdev;

	dev = zero_extend_signed_to_ull(st->st_dev);
	rdev = zero_extend_signed_to_ull(st->st_rdev);
	printf("{st_dev=%s", sprint_makedev(dev));
	printf(", st_ino=%llu", zero_extend_signed_to_ull(st->st_ino));
	printf(", st_mode=");
	print_st_mode(st->st_mode);
	printf(", st_nlink=%llu", zero_extend_signed_to_ull(st->st_nlink));
	printf(", st_uid=%llu", zero_extend_signed_to_ull(st->st_uid));
	printf(", st_gid=%llu", zero_extend_signed_to_ull(st->st_gid));
#  if OLD_STAT
	printf(", st_blksize=0, st_blocks=0");
#  else /* !OLD_STAT */
	printf(", st_blksize=%llu", zero_extend_signed_to_ull(st->st_blksize));
	printf(", st_blocks=%llu", zero_extend_signed_to_ull(st->st_blocks));
#  endif /* OLD_STAT */

	switch (st->st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
		printf(", st_rdev=%s", sprint_makedev(rdev));
		break;
	default:
		printf(", st_size=%llu", zero_extend_signed_to_ull(st->st_size));
	}

#  if defined(HAVE_STRUCT_STAT_ST_MTIME_NSEC) && !OLD_STAT
#   define TIME_NSEC(val)	zero_extend_signed_to_ull(val)
#   define HAVE_NSEC		1
#  else
#   define TIME_NSEC(val)	0ULL
#   define HAVE_NSEC		0
#  endif

#  define PRINT_ST_TIME(field)							\
	do {									\
		printf(", st_" #field "=%lld",					\
		       sign_extend_unsigned_to_ll(st->st_ ## field));		\
		print_time_t_nsec(sign_extend_unsigned_to_ll(st->st_ ## field),	\
				  TIME_NSEC(st->st_ ## field ## _nsec), 1);	\
		if (HAVE_NSEC)							\
			printf(", st_" #field "_nsec=%llu",			\
			       TIME_NSEC(st->st_ ## field ## _nsec));		\
	} while (0)

	PRINT_ST_TIME(atime);
	PRINT_ST_TIME(mtime);
	PRINT_ST_TIME(ctime);
	printf("}");
}

# else /* !IS_STATX */

static void
print_stat(const STRUCT_STAT *st)
{
#  define PRINT_FIELD_U32_UID(field)					\
	do {								\
		if (st->field == (uint32_t) -1)				\
			printf(", %s=-1", #field);			\
		else							\
			printf(", %s=%llu", #field,			\
			       (unsigned long long) st->field);		\
	} while (0)

#  define PRINT_FIELD_TIME(field)						\
	do {									\
		printf(", %s={tv_sec=%lld, tv_nsec=%u}",			\
		       #field, (long long) st->field.tv_sec,			\
		       (unsigned) st->field.tv_nsec);				\
		print_time_t_nsec(st->field.tv_sec,				\
				  zero_extend_signed_to_ull(st->field.tv_nsec),	\
				  1);						\
	} while (0)

	printf("{stx_mask=");
	printflags(statx_masks, st->stx_mask, "STATX_???");

	PRINT_FIELD_U(", ", *st, stx_blksize);

	printf(", stx_attributes=");
	printflags(statx_attrs, st->stx_attributes, "STATX_ATTR_???");

	PRINT_FIELD_U(", ", *st, stx_nlink);
	PRINT_FIELD_U32_UID(stx_uid);
	PRINT_FIELD_U32_UID(stx_gid);

	printf(", stx_mode=");
	print_st_mode(st->stx_mode);

	PRINT_FIELD_U(", ", *st, stx_ino);
	PRINT_FIELD_U(", ", *st, stx_size);
	PRINT_FIELD_U(", ", *st, stx_blocks);

	printf(", stx_attributes_mask=");
	printflags(statx_attrs, st->stx_attributes_mask, "STATX_ATTR_???");

	PRINT_FIELD_TIME(stx_atime);
	PRINT_FIELD_TIME(stx_btime);
	PRINT_FIELD_TIME(stx_ctime);
	PRINT_FIELD_TIME(stx_mtime);
	PRINT_FIELD_U(", ", *st, stx_rdev_major);
	PRINT_FIELD_U(", ", *st, stx_rdev_minor);
	PRINT_FIELD_U(", ", *st, stx_dev_major);
	PRINT_FIELD_U(", ", *st, stx_dev_minor);
	printf("}");
}

# endif /* !IS_STATX */

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
# if IS_FSTAT
	skip_if_unavailable("/proc/self/fd/");
# else
	static const char full[] = "/dev/full";
# endif
	static const char sample[] = "stat.sample";
	TAIL_ALLOC_OBJECT_CONST_PTR(STRUCT_STAT, st);

	int rc;

	rc = create_sample(sample, SAMPLE_SIZE);
	if (rc)
		return rc;

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
		if (errno != EOVERFLOW) {
			rc = (errno == ENOSYS) ? 77 : 1;
			perror(TEST_SYSCALL_STR);
			return rc;
		}
	}

# if IS_STATX
#  define ST_SIZE_FIELD stx_size
# else
#  define ST_SIZE_FIELD st_size
# endif
	if (!rc && zero_extend_signed_to_ull(SAMPLE_SIZE) !=
	    zero_extend_signed_to_ull(st->ST_SIZE_FIELD)) {
		fprintf(stderr, "Size mismatch: "
				"requested size(%llu) != st_size(%llu)\n",
			zero_extend_signed_to_ull(SAMPLE_SIZE),
			zero_extend_signed_to_ull(st->ST_SIZE_FIELD));
		fprintf(stderr, "The most likely reason for this is incorrect"
				" definition of %s.\n"
				"Here is some diagnostics that might help:\n",
			STRUCT_STAT_STR);

# define LOG_STAT_OFFSETOF_SIZEOF(object, member)			\
		fprintf(stderr, "offsetof(%s, %s) = %zu"		\
				", sizeof(%s) = %zu\n",			\
				STRUCT_STAT_STR, #member,		\
				offsetof(STRUCT_STAT, member),		\
				#member, sizeof((object).member))

# if IS_STATX
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_mask);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_blksize);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_attributes);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_nlink);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_uid);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_gid);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_mode);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_ino);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_size);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_blocks);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_attributes_mask);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_atime);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_btime);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_ctime);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_mtime);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_rdev_major);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_rdev_minor);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_dev_major);
		LOG_STAT_OFFSETOF_SIZEOF(*st, stx_dev_minor);
# else
		LOG_STAT_OFFSETOF_SIZEOF(*st, st_dev);
		LOG_STAT_OFFSETOF_SIZEOF(*st, st_ino);
		LOG_STAT_OFFSETOF_SIZEOF(*st, st_mode);
		LOG_STAT_OFFSETOF_SIZEOF(*st, st_nlink);
		LOG_STAT_OFFSETOF_SIZEOF(*st, st_uid);
		LOG_STAT_OFFSETOF_SIZEOF(*st, st_gid);
		LOG_STAT_OFFSETOF_SIZEOF(*st, st_rdev);
		LOG_STAT_OFFSETOF_SIZEOF(*st, st_size);
#  if !OLD_STAT
		LOG_STAT_OFFSETOF_SIZEOF(*st, st_blksize);
		LOG_STAT_OFFSETOF_SIZEOF(*st, st_blocks);
#  endif /* !OLD_STAT */

# endif /* IS_STATX */

		return 1;
	}

	PRINT_SYSCALL_HEADER(sample);
	if (rc)
		printf("%p", st);
	else
		print_stat(st);
	PRINT_SYSCALL_FOOTER(rc);

# if IS_STATX

#  define INVOKE()					\
	do {						\
		rc = TEST_SYSCALL_INVOKE(sample, st);	\
		PRINT_SYSCALL_HEADER(sample);		\
		if (rc)					\
			printf("%p", st);		\
		else					\
			print_stat(st);			\
		PRINT_SYSCALL_FOOTER(rc);		\
	} while (0)

#  define SET_FLAGS_INVOKE(flags, flags_str)			\
	do {							\
		TEST_SYSCALL_STATX_FLAGS = flags;		\
		TEST_SYSCALL_STATX_FLAGS_STR = flags_str;	\
		INVOKE();					\
	} while (0)

#  define SET_MASK_INVOKE(mask, mask_str)			\
	do {							\
		TEST_SYSCALL_STATX_MASK = mask;			\
		TEST_SYSCALL_STATX_MASK_STR = mask_str;		\
		INVOKE();					\
	} while (0)

	unsigned old_flags = TEST_SYSCALL_STATX_FLAGS;
	const char *old_flags_str = TEST_SYSCALL_STATX_FLAGS_STR;
	unsigned old_mask = TEST_SYSCALL_STATX_MASK;
	const char *old_mask_str = TEST_SYSCALL_STATX_MASK_STR;

	SET_FLAGS_INVOKE(AT_SYMLINK_FOLLOW | 0xffff0000U,
		"AT_STATX_SYNC_AS_STAT|AT_SYMLINK_FOLLOW|0xffff0000");

	SET_FLAGS_INVOKE(AT_STATX_SYNC_TYPE,
		"AT_STATX_FORCE_SYNC|AT_STATX_DONT_SYNC");

	SET_FLAGS_INVOKE(0xffffff,
		"AT_STATX_FORCE_SYNC|AT_STATX_DONT_SYNC|AT_SYMLINK_NOFOLLOW|"
		"AT_REMOVEDIR|AT_SYMLINK_FOLLOW|AT_NO_AUTOMOUNT|AT_EMPTY_PATH|"
		"AT_RECURSIVE|0xff00ff");

	/* We're done playing with flags. */
	TEST_SYSCALL_STATX_FLAGS = old_flags;
	TEST_SYSCALL_STATX_FLAGS_STR = old_flags_str;

	SET_MASK_INVOKE(0, "0");
	SET_MASK_INVOKE(0xffffe000U, "0xffffe000 /* STATX_??? */");

	SET_MASK_INVOKE(0xfffffffbU,
		"STATX_TYPE|STATX_MODE|STATX_UID|STATX_GID|STATX_ATIME|"
		"STATX_MTIME|STATX_CTIME|STATX_INO|STATX_SIZE|STATX_BLOCKS|"
		"STATX_BTIME|STATX_MNT_ID|0xffffe000");

	SET_MASK_INVOKE(STATX_UID, "STATX_UID");

	/* ...and with mask. */
	TEST_SYSCALL_STATX_MASK = old_mask;
	TEST_SYSCALL_STATX_MASK_STR = old_mask_str;

# endif /* IS_STATX */

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_FTRUNCATE && HAVE_FUTIMENS")

#endif
