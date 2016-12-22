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

#include "tests.h"

#ifdef HAVE_SYS_XATTR_H

# include <stdio.h>
# include <sys/xattr.h>

# ifndef XATTR_SIZE_MAX
#  define XATTR_SIZE_MAX 65536
# endif

#define DEFAULT_STRLEN 32

int
main(void)
{
	static const char name[] = "strace.test";
	static const char c_value[] = "foo\0bar";
	static const char q_value[] = "foo\\0bar";

	const char *const z_value = tail_memdup(c_value, sizeof(c_value));
	char *const efault = tail_alloc(1) + 1;
	const char *const value = tail_memdup(c_value, sizeof(c_value) - 1);
	char *const big = tail_alloc(XATTR_SIZE_MAX + 1);
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

	rc = lsetxattr(".", name, value, sizeof(c_value) - 1, XATTR_CREATE);
	printf("lsetxattr(\".\", \"%s\", \"%s\", %u, XATTR_CREATE) = %s\n",
	       name, q_value, (unsigned) sizeof(c_value) - 1, sprintrc(rc));

	rc = fgetxattr(-1, name, efault, 4);
	printf("fgetxattr(-1, \"%s\", %p, 4) = %s\n",
	       name, efault, sprintrc(rc));

	rc = getxattr(".", name, big, XATTR_SIZE_MAX + 1);
	printf("getxattr(\".\", \"%s\", %p, %u) = %s\n",
	       name, big, XATTR_SIZE_MAX + 1, sprintrc(rc));

	rc = lgetxattr(".", name, big + 1, XATTR_SIZE_MAX);
	printf("lgetxattr(\".\", \"%s\", %p, %u) = %s\n",
	       name, big + 1, XATTR_SIZE_MAX, sprintrc(rc));

	rc = flistxattr(-1, efault, 4);
	printf("flistxattr(-1, %p, 4) = %s\n", efault, sprintrc(rc));

	rc = llistxattr("", efault + 1, 4);
	printf("llistxattr(\"\", %p, 4) = %s\n", efault + 1, sprintrc(rc));

	rc = listxattr(".", big, 0);
	printf("listxattr(\".\", %p, 0) = %s\n", big, sprintrc(rc));

	rc = listxattr(".", big, XATTR_SIZE_MAX + 1);
	errstr = sprintrc(rc);
	printf("listxattr(\".\", ");
	if (rc < 0)
		printf("%p", big);
	else {
		const int ellipsis = rc > DEFAULT_STRLEN;

		putchar('"');
		print_quoted_memory(big, ellipsis ? DEFAULT_STRLEN : rc);
		putchar('"');
		if (ellipsis)
			fputs("...", stdout);
	}
	printf(", %u) = %s\n", XATTR_SIZE_MAX + 1, errstr);

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
