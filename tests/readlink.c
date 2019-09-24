/*
 * Copyright (c) 2015 Gleb Fotengauer-Malinovskiy <glebfm@altlinux.org>
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_readlink

# include <stdio.h>
# include <unistd.h>

# define PREFIX "test.readlink"
# define TARGET (PREFIX ".target")
# define LINKPATH (PREFIX ".link")

int
main(void)
{
	const char * const fname = tail_memdup(LINKPATH, sizeof(LINKPATH));
	const char * const hex_fname =
		hexquote_strndup(fname, sizeof(LINKPATH) - 1);

	const unsigned int size = sizeof(TARGET) - 1;
	char * const buf = tail_alloc(size);

	(void) unlink(fname);

	long rc = syscall(__NR_readlink, fname, buf, size);
	printf("readlink(\"%s\", %p, %u) = -1 ENOENT (%m)\n",
	       hex_fname, buf, size);

	if (symlink(TARGET, fname))
		perror_msg_and_fail("symlink");

	rc = syscall(__NR_readlink, fname, buf, size);
	if (rc < 0) {
		perror("readlink");
		(void) unlink(fname);
		return 77;
	}
	const char * const hex_buf = hexquote_strndup(buf, size);
	printf("readlink(\"%s\", \"%s\", %u) = %u\n",
	       hex_fname, hex_buf, size, size);

	if (unlink(fname))
		perror_msg_and_fail("unlink");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_readlink")

#endif
