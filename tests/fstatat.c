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

#if defined FSTATAT_NAME && defined HAVE_FSTATAT \
 && defined HAVE_FTRUNCATE && defined HAVE_FUTIMENS

# include <errno.h>
# include <fcntl.h>
# include <stdio.h>
# include <time.h>
# include <unistd.h>
# include <sys/stat.h>

#if defined MAJOR_IN_SYSMACROS
# include <sys/sysmacros.h>
#elif defined MAJOR_IN_MKDEV
# include <sys/mkdev.h>
#else
# include <sys/types.h>
#endif

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

static void
print_stat(const struct stat *st)
{
	printf("{st_dev=makedev(%u, %u)",
	       (unsigned int) major(st->st_dev),
	       (unsigned int) minor(st->st_dev));
	printf(", st_ino=%Lu", (unsigned long long) st->st_ino);
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
		printf(", st_size=%Lu", (unsigned long long) st->st_size);
	}

	printf(", st_atime=");
	print_time(st->st_atime);
	if (st->st_atim.tv_nsec)
		printf(".%09lu", (unsigned long) st->st_atim.tv_nsec);
	printf(", st_mtime=");
	print_time(st->st_mtime);
	if (st->st_mtim.tv_nsec)
		printf(".%09lu", (unsigned long) st->st_mtim.tv_nsec);
	printf(", st_ctime=");
	print_time(st->st_ctime);
	if (st->st_ctim.tv_nsec)
		printf(".%09lu", (unsigned long) st->st_ctim.tv_nsec);
	printf("}");
}

int
main(void)
{
	static const char sample[] = FSTATAT_NAME ".sample";
	static const struct timespec ts[] = {
		{-10843, 135}, {-10841, 246}
	};
	const off_t size = 46118400291;
	struct stat st;

	(void) close(0);
	if (open(sample, O_RDWR | O_CREAT | O_TRUNC, 0640)) {
		perror(sample);
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
	if (fstatat(AT_FDCWD, sample, &st, AT_SYMLINK_NOFOLLOW)) {
		perror("fstatat");
		return 77;
	}
	(void) unlink(sample);

	printf("%s(AT_FDCWD, \"%s\", ", FSTATAT_NAME, sample);
	print_stat(&st);
	puts(", AT_SYMLINK_NOFOLLOW) = 0");

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
