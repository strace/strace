/*
 * Check decoding of readlinkat syscall.
 *
 * Copyright (c) 2015 Gleb Fotengauer-Malinovskiy <glebfm@altlinux.org>
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

#define PREFIX "test.readlinkat"
#define TARGET (PREFIX ".target")
#define LINKPATH (PREFIX ".link")

int
main(void)
{
	const char * const fname = tail_memdup(LINKPATH, sizeof(LINKPATH));
	const char * const hex_fname =
		hexquote_strndup(fname, sizeof(LINKPATH) - 1);

	const unsigned int size = sizeof(TARGET) - 1;
	char * const buf = tail_alloc(size);

	(void) unlink(fname);

	long rc = syscall(__NR_readlinkat, -100, fname, buf, size);
	printf("readlinkat(AT_FDCWD, \"%s\", %p, %u)" RVAL_ENOENT,
	       hex_fname, buf, size);

	if (symlink(TARGET, fname))
		perror_msg_and_fail("symlink");

	rc = syscall(__NR_readlinkat, -100, fname, buf, size);
	if (rc < 0) {
		perror("readlinkat");
		(void) unlink(fname);
		return 77;
	}
	const char * const hex_buf = hexquote_strndup(buf, size);
	printf("readlinkat(AT_FDCWD, \"%s\", \"%s\", %u) = %u\n",
	       hex_fname, hex_buf, size, size);

	if (unlink(fname))
		perror_msg_and_fail("unlink");

	puts("+++ exited with 0 +++");
	return 0;
}
