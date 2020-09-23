/*
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_mknod

# include <stdio.h>
# include <sys/stat.h>
# include <sys/sysmacros.h>
# include <unistd.h>

static const char *sample;

static long
call_mknod(unsigned short mode, unsigned long dev)
{
	unsigned long lmode = (unsigned long) 0xffffffffffff0000ULL | mode;
	return syscall(__NR_mknod, sample, lmode, dev);
}

int
main(int ac, char **av)
{
	unsigned long dev = (unsigned long) 0xdeadbeefbadc0dedULL;
	sample = av[0];

	long rc = call_mknod(0, dev);
	printf("mknod(\"%s\", 000) = %s\n",
	       sample, sprintrc(rc));

	rc = call_mknod(0xffff, dev);
	printf("mknod(\"%s\", %#03ho) = %s\n",
	       sample, (unsigned short) -1, sprintrc(rc));

	rc = call_mknod(S_IFREG, 0);
	printf("mknod(\"%s\", S_IFREG|000) = %s\n",
	       sample, sprintrc(rc));

	rc = call_mknod(S_IFDIR | 06, 0);
	printf("mknod(\"%s\", S_IFDIR|006) = %s\n",
	       sample, sprintrc(rc));

	rc = call_mknod(S_IFLNK | 060, 0);
	printf("mknod(\"%s\", S_IFLNK|060) = %s\n",
	       sample, sprintrc(rc));

	rc = call_mknod(S_IFIFO | 0600, 0);
	printf("mknod(\"%s\", S_IFIFO|0600) = %s\n",
	       sample, sprintrc(rc));

	dev = (unsigned long) 0xdeadbeef00000000ULL | makedev(1, 7);

	rc = call_mknod(S_IFCHR | 024, dev);
	printf("mknod(\"%s\", S_IFCHR|024, makedev(0x1, 0x7)) = %s\n",
	       sample, sprintrc(rc));

	const unsigned short mode = (0xffff & ~S_IFMT) | S_IFBLK;
	dev = (unsigned long) 0xdeadbeefbadc0dedULL;

	rc = call_mknod(mode, dev);
	printf("mknod(\"%s\", S_IFBLK|S_ISUID|S_ISGID|S_ISVTX|%#03ho"
	       ", makedev(%#x, %#x)) = %s\n",
	       sample, (short) (mode & ~(S_IFMT|S_ISUID|S_ISGID|S_ISVTX)),
	       major((unsigned) dev), minor((unsigned) dev),
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mknod")

#endif
