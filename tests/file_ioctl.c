/*
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_LINUX_FIEMAP_H

# include <stdio.h>
# include <sys/ioctl.h>
# include <linux/types.h>
# include <linux/fiemap.h>
# include <linux/fs.h>
# include "xlat.h"
# include "xlat/fiemap_flags.h"

static void
test_fiemap(void)
{
	(void) tail_alloc(1);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct fiemap, args);

	printf("ioctl(-1, FS_IOC_FIEMAP, {fm_start=%" PRI__u64
	       ", fm_length=%" PRI__u64", fm_flags=",
	       args->fm_start, args->fm_length);
	printflags(fiemap_flags, args->fm_flags, "FIEMAP_FLAG_???");
	printf(", fm_extent_count=%u})", args->fm_extent_count);
	ioctl(-1, FS_IOC_FIEMAP, args);
	printf(" = -1 EBADF (%m)\n");

	/* The live version of this test is in btrfs.c */
}

/* clone and dedupe ioctls are in btrfs.c since they originated there */

int
main(int argc, char *argv[])
{
	test_fiemap();

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_LINUX_FIEMAP_H")

#endif
