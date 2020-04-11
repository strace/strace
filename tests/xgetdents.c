/*
 * Check decoding of getdents and getdents64 syscalls.
 *
 * Copyright (c) 2015-2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "kernel_dirent.h"
#include "print_fields.h"

#if VERBOSE
static const char *
str_d_type(const unsigned char d_type)
{
	switch (d_type) {
		case DT_DIR:
			return "DT_DIR";
		case DT_REG:
			return "DT_REG";
		default:
			return "DT_UNKNOWN";
	}
}

static void
print_dirent(const kernel_dirent_type *d);
#endif

static const char *errstr;

static long
k_getdents(const unsigned int fd,
	   const void *dirp,
	   const unsigned int count)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | fd;
	const kernel_ulong_t arg2 = (uintptr_t) dirp;
	const kernel_ulong_t arg3 = fill | count;
	const long rc = syscall(NR_getdents, arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

static void
ls(int fd, char *buf, unsigned int size)
{
	long rc;
	while ((rc = k_getdents(fd, buf, size))) {
		if (rc < 0)
			perror_msg_and_skip(STR_getdents);
		printf("%s(%d, ", STR_getdents, fd);
#if VERBOSE
		printf("[");
#else
		unsigned long entries = 0;
#endif
		kernel_dirent_type *d;
		for (long i = 0; i < rc; i += d->d_reclen) {
			d = (kernel_dirent_type *) &buf[i];
#if VERBOSE
			if (i)
				printf(", ");
			print_dirent(d);
#else
			++entries;
#endif
		}
#if VERBOSE
		printf("]");
#else
		printf("%p /* %lu entries */", buf, entries);
#endif
		printf(", %u) = %ld\n", size, rc);
	}
#if VERBOSE
	printf("%s(%d, [], %u) = 0\n", STR_getdents, fd, size);
#else
	printf("%s(%d, %p /* 0 entries */, %u) = 0\n",
	       STR_getdents, fd, buf, size);
#endif
}

int
main(void)
{
	static const char dot[] = ".";
	static const char dot_dot[] = "..";
	static const char dname[] = STR_getdents
#if VERBOSE
		"-v"
#endif
		".test.tmp.dir";
	static const char fname[] =
		"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
		"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
		"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
		"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
		"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
		"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
		"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
		"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nZ";

	if (mkdir(dname, 0700))
		perror_msg_and_fail("mkdir: %s", dname);
	if (chdir(dname))
		perror_msg_and_fail("chdir: %s", dname);
	int fd = creat(fname, 0600);
	if (fd < 0)
		perror_msg_and_fail("creat: %s", fname);
	if (close(fd))
		perror_msg_and_fail("close: %s", fname);
	fd = open(dot, O_RDONLY | O_DIRECTORY);
	if (fd < 0)
		perror_msg_and_fail("open: %s", dot);

	unsigned int count = 0xdeadbeefU;
	k_getdents(-1U, NULL, count);
	printf("%s(-1, NULL, %u) = %s\n", STR_getdents, count, errstr);

	static char buf[8192];
	ls(fd, buf, sizeof(buf));

	if (unlink(fname))
		perror_msg_and_fail("unlink: %s", fname);
	if (chdir(dot_dot))
		perror_msg_and_fail("chdir: %s", dot_dot);
	if (rmdir(dname))
		perror_msg_and_fail("rmdir: %s", dname);

	puts("+++ exited with 0 +++");
	return 0;
}
