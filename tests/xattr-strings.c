/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_SYS_XATTR_H

# include <stdio.h>
# include <sys/xattr.h>

int
main(void)
{
	static const char name[] = "strace.test";
	static const char c_value[] = "foobar";

	const char *const z_value = tail_memdup(c_value, sizeof(c_value));
	long rc;

	rc = fsetxattr(-1, name, z_value, sizeof(c_value), XATTR_REPLACE);
	printf("fsetxattr(-1, \"%.*s\"..., \"%.*s\"..., %u, XATTR_REPLACE) = %s\n",
	       4, name, 4, c_value, (unsigned) sizeof(c_value), sprintrc(rc));


	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_XATTR_H")

#endif
