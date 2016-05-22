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

# include <assert.h>
# include <stdio.h>
# include <sys/xattr.h>

# ifndef XATTR_SIZE_MAX
#  define XATTR_SIZE_MAX 65536
# endif

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

	assert(fsetxattr(-1, 0, 0, 0, XATTR_CREATE) == -1);
	printf("fsetxattr(-1, NULL, NULL, 0, XATTR_CREATE) = -1 %s (%m)\n",
	       errno2name());

	assert(fsetxattr(-1, 0, z_value, 0, XATTR_CREATE) == -1);
	printf("fsetxattr(-1, NULL, \"\", 0, XATTR_CREATE) = -1 %s (%m)\n",
	       errno2name());

	assert(fsetxattr(-1, name, big, XATTR_SIZE_MAX + 1, XATTR_CREATE) == -1);
	printf("fsetxattr(-1, \"%s\", %p, %u, XATTR_CREATE)"
	       " = -1 %s (%m)\n",
	       name, big, XATTR_SIZE_MAX + 1, errno2name());

	assert(fsetxattr(-1, name, value, sizeof(c_value), XATTR_CREATE) == -1);
	printf("fsetxattr(-1, \"%s\", %p, %u, XATTR_CREATE)"
	       " = -1 %s (%m)\n",
	       name, value, (unsigned) sizeof(c_value), errno2name());

	assert(fsetxattr(-1, name, z_value, sizeof(c_value), XATTR_REPLACE) == -1);
	printf("fsetxattr(-1, \"%s\", \"%s\", %u, XATTR_REPLACE)"
	       " = -1 %s (%m)\n",
	       name, q_value, (unsigned) sizeof(c_value), errno2name());

	assert(fsetxattr(-1, name, value, sizeof(c_value) - 1, XATTR_CREATE|XATTR_REPLACE) == -1);
	printf("fsetxattr(-1, \"%s\", \"%s\", %u, XATTR_CREATE|XATTR_REPLACE)"
	       " = -1 %s (%m)\n",
	       name, q_value, (unsigned) sizeof(c_value) - 1, errno2name());

	assert(setxattr(".", name, z_value, sizeof(c_value), XATTR_CREATE) == -1);
	printf("setxattr(\".\", \"%s\", \"%s\", %u, XATTR_CREATE)"
	       " = -1 %s (%m)\n",
	       name, q_value, (unsigned) sizeof(c_value), errno2name());

	assert(lsetxattr(".", name, value, sizeof(c_value) - 1, XATTR_CREATE) == -1);
	printf("lsetxattr(\".\", \"%s\", \"%s\", %u, XATTR_CREATE)"
	       " = -1 %s (%m)\n",
	       name, q_value, (unsigned) sizeof(c_value) - 1, errno2name());

	assert(fgetxattr(-1, name, efault, 4) == -1);
	printf("fgetxattr(-1, \"%s\", %p, 4) = -1 %s (%m)\n",
	       name, efault, errno2name());

	assert(getxattr(".", name, big, XATTR_SIZE_MAX + 1) == -1);
	printf("getxattr(\".\", \"%s\", %p, %u) = -1 %s (%m)\n",
	       name, big, XATTR_SIZE_MAX + 1, errno2name());

	assert(lgetxattr(".", name, big + 1, XATTR_SIZE_MAX) == -1);
	printf("lgetxattr(\".\", \"%s\", %p, %u) = -1 %s (%m)\n",
	       name, big + 1, XATTR_SIZE_MAX, errno2name());

	assert(flistxattr(-1, efault, 4) == -1);
	printf("flistxattr(-1, %p, 4) = -1 %s (%m)\n",
	       efault, errno2name());

	assert(llistxattr("", efault + 1, 4) == -1);
	printf("llistxattr(\"\", %p, 4) = -1 %s (%m)\n",
	       efault + 1, errno2name());

	ssize_t rc = listxattr(".", big, 0);
	if (rc < 0)
		printf("listxattr(\".\", %p, 0) = -1 %s (%m)\n",
		       big, errno2name());
	else
		printf("listxattr(\".\", %p, 0) = %ld\n",
		       big, (long) rc);

	rc = listxattr(".", big, XATTR_SIZE_MAX + 1);
	if (rc < 0)
		printf("listxattr(\".\", %p, %u) = -1 %s (%m)\n",
		       big, XATTR_SIZE_MAX + 1, errno2name());
	else {
		printf("listxattr(\".\", \"");
		print_quoted_memory(big, rc);
		printf("\", %u) = %ld\n", XATTR_SIZE_MAX + 1, (long) rc);
	}

	assert(fremovexattr(-1, name) == -1);
	printf("fremovexattr(-1, \"%s\") = -1 %s (%m)\n",
	       name, errno2name());

	assert(removexattr(".", name) == -1);
	printf("removexattr(\".\", \"%s\") = -1 %s (%m)\n",
	       name, errno2name());

	assert(lremovexattr(".", name) == -1);
	printf("lremovexattr(\".\", \"%s\") = -1 %s (%m)\n",
	       name, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_XATTR_H")

#endif
