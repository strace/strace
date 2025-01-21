/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_SYS_XATTR_H

# include <stdio.h>
# include <sys/xattr.h>

# ifndef XATTR_SIZE_MAX
#  define XATTR_SIZE_MAX 65536
# endif

int
main(void)
{
	static const char name[] = "user.strace.test.xattr";
	static const char c_value[] = "foo\0bar";
	static const char q_value[] = "foo\\0bar";

	const char *const z_value = tail_memdup(c_value, sizeof(c_value));
	char *const efault = tail_alloc(1) + 1;
	const char *const value = tail_memdup(c_value, sizeof(c_value) - 1);
	const unsigned int big_size = XATTR_SIZE_MAX;
	char *const big = tail_alloc(big_size);
	long rc;
	const char *errstr;

	rc = fsetxattr(-1, 0, 0, 0, XATTR_CREATE);
	printf("fsetxattr(-1, NULL, NULL, 0, XATTR_CREATE) = %s\n",
	       sprintrc(rc));

	rc = fsetxattr(-1, 0, z_value, 0, XATTR_CREATE);
	printf("fsetxattr(-1, NULL, \"\", 0, XATTR_CREATE) = %s\n",
	       sprintrc(rc));

	rc = fsetxattr(-1, name, big, XATTR_SIZE_MAX + 1, XATTR_CREATE);
	printf("fsetxattr(-1, \"%s\", %p, %u, XATTR_CREATE) = %s\n",
	       name, big, XATTR_SIZE_MAX + 1, sprintrc(rc));

	fill_memory(big, big_size);
	rc = fsetxattr(-1, name, big, big_size, XATTR_CREATE);
	errstr = sprintrc(rc);
	printf("fsetxattr(-1, \"%s\", ", name);
	print_quoted_memory(big, DEFAULT_STRLEN);
	fputs("...", stdout);
	printf(", %u, XATTR_CREATE) = %s\n", big_size, errstr);

	rc = fsetxattr(-1, name, value, sizeof(c_value), XATTR_CREATE);
	printf("fsetxattr(-1, \"%s\", %p, %u, XATTR_CREATE) = %s\n",
	       name, value, (unsigned) sizeof(c_value), sprintrc(rc));

	rc = fsetxattr(-1, name, z_value, sizeof(c_value), XATTR_REPLACE);
	printf("fsetxattr(-1, \"%s\", \"%s\", %u, XATTR_REPLACE) = %s\n",
	       name, q_value, (unsigned) sizeof(c_value), sprintrc(rc));

	rc = fsetxattr(-1, name, value, sizeof(c_value) - 1, XATTR_CREATE|XATTR_REPLACE);
	printf("fsetxattr(-1, \"%s\", \"%s\", %u, XATTR_CREATE|XATTR_REPLACE)"
	       " = %s\n",
	       name, q_value, (unsigned) sizeof(c_value) - 1, sprintrc(rc));

	rc = setxattr(".", name, z_value, sizeof(c_value), XATTR_CREATE);
	printf("setxattr(\".\", \"%s\", \"%s\", %u, XATTR_CREATE) = %s\n",
	       name, q_value, (unsigned) sizeof(c_value), sprintrc(rc));

	rc = lsetxattr(".", name, value, sizeof(c_value) - 1, XATTR_REPLACE);
	printf("lsetxattr(\".\", \"%s\", \"%s\", %u, XATTR_REPLACE) = %s\n",
	       name, q_value, (unsigned) sizeof(c_value) - 1, sprintrc(rc));

	rc = fgetxattr(-1, name, efault, 4);
	printf("fgetxattr(-1, \"%s\", %p, 4) = %s\n",
	       name, efault, sprintrc(rc));

	rc = getxattr(".", name, big, 0);
	printf("getxattr(\".\", \"%s\", %p, 0) = %s\n",
	       name, big, sprintrc(rc));

	rc = lgetxattr(".", name, big, big_size);
	errstr = sprintrc(rc);
	printf("lgetxattr(\".\", \"%s\", ", name);
	if (rc < 0)
		printf("%p", big);
	else
		printf("\"%s\"", q_value);
	printf(", %u) = %s\n", big_size, errstr);

	rc = flistxattr(-1, efault, 4);
	printf("flistxattr(-1, %p, 4) = %s\n", efault, sprintrc(rc));

	rc = listxattr(".", big, 0);
	printf("listxattr(\".\", %p, 0) = %s\n", big, sprintrc(rc));

	rc = llistxattr(".", big, big_size);
	errstr = sprintrc(rc);
	printf("llistxattr(\".\", ");
	if (rc < 0)
		printf("%p", big);
	else {
		const int ellipsis = rc > DEFAULT_STRLEN;

		print_quoted_memory(big, ellipsis ? DEFAULT_STRLEN : rc);
		if (ellipsis)
			fputs("...", stdout);
	}
	printf(", %u) = %s\n", big_size, errstr);

	rc = fremovexattr(-1, name);
	printf("fremovexattr(-1, \"%s\") = %s\n", name, sprintrc(rc));

	rc = removexattr(".", name);
	printf("removexattr(\".\", \"%s\") = %s\n", name, sprintrc(rc));

	rc = lremovexattr(".", name);
	printf("lremovexattr(\".\", \"%s\") = %s\n", name, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_XATTR_H")

#endif
