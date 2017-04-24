/*
 * This file is part of execve strace test.
 *
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
#include <stdio.h>
#include <unistd.h>

#define FILENAME "test.execve\nfilename"
#define Q_FILENAME "test.execve\\nfilename"

static const char * const argv[] = {
	FILENAME, "first", "second", (const char *) -1L,
	(const char *) -2L, (const char *) -3L
};
static const char * const q_argv[] = {
	Q_FILENAME, "first", "second"
};

static const char * const envp[] = {
	"foobar=1", "foo\nbar=2", (const char *) -1L,
	(const char *) -2L, (const char *) -3L
};
static const char * const q_envp[] = {
	"foobar=1", "foo\\nbar=2"
};

int
main(void)
{
	char ** const tail_argv = tail_memdup(argv, sizeof(argv));
	char ** const tail_envp = tail_memdup(envp, sizeof(envp));

	execve(FILENAME, tail_argv, tail_envp);
	printf("execve(\"%s\""
	       ", [\"%s\", \"%s\", \"%s\", %p, %p, %p, ???]"
#if VERBOSE
	       ", [\"%s\", \"%s\", %p, %p, %p, ???]"
#else
	       ", %p /* 5 vars, unterminated */"
#endif
	       ") = -1 ENOENT (%m)\n",
	       Q_FILENAME, q_argv[0], q_argv[1], q_argv[2],
	       argv[3], argv[4], argv[5]
#if VERBOSE
	       , q_envp[0], q_envp[1], envp[2], envp[3], envp[4]
#else
	       , tail_envp
#endif
	       );

	tail_argv[ARRAY_SIZE(q_argv)] = NULL;
	tail_envp[ARRAY_SIZE(q_envp)] = NULL;

	execve(FILENAME, tail_argv, tail_envp);
	printf("execve(\"%s\", [\"%s\", \"%s\", \"%s\"]"
#if VERBOSE
	       ", [\"%s\", \"%s\"]"
#else
	       ", %p /* 2 vars */"
#endif
	       ") = -1 ENOENT (%m)\n",
	       Q_FILENAME, q_argv[0], q_argv[1], q_argv[2]
#if VERBOSE
	       , q_envp[0], q_envp[1]
#else
	       , tail_envp
#endif
	       );

	execve(FILENAME, tail_argv + 2, tail_envp + 1);
	printf("execve(\"%s\", [\"%s\"]"
#if VERBOSE
	       ", [\"%s\"]"
#else
	       ", %p /* 1 var */"
#endif
	       ") = -1 ENOENT (%m)\n",
	       Q_FILENAME, q_argv[2]
#if VERBOSE
	       , q_envp[1]
#else
	       , tail_envp + 1
#endif
	       );

	TAIL_ALLOC_OBJECT_CONST_PTR(char *, empty);
	char **const efault = empty + 1;
	*empty = NULL;

	execve(FILENAME, empty, empty);
	printf("execve(\"%s\", []"
#if VERBOSE
	       ", []"
#else
	       ", %p /* 0 vars */"
#endif
	       ") = -1 ENOENT (%m)\n", Q_FILENAME
#if !VERBOSE
	       , empty
#endif
	       );

	char str_a[] = "012345678901234567890123456789012";
	char str_b[] = "_abcdefghijklmnopqrstuvwxyz()[]{}";
#define DEFAULT_STRLEN ((unsigned int) sizeof(str_a) - 2)
	char **const a = tail_alloc(sizeof(*a) * (DEFAULT_STRLEN + 2));
	char **const b = tail_alloc(sizeof(*b) * (DEFAULT_STRLEN + 2));
	unsigned int i;
	for (i = 0; i <= DEFAULT_STRLEN; ++i) {
		a[i] = &str_a[i];
		b[i] = &str_b[i];
	}
	a[i] = b[i] = NULL;

	execve(FILENAME, a, b);
	printf("execve(\"%s\", [\"%.*s\"...", Q_FILENAME, DEFAULT_STRLEN, a[0]);
	for (i = 1; i < DEFAULT_STRLEN; ++i)
		printf(", \"%s\"", a[i]);
#if VERBOSE
	printf(", \"%s\"", a[i]);
#else
	printf(", ...");
#endif
#if VERBOSE
	printf("], [\"%.*s\"...", DEFAULT_STRLEN, b[0]);
	for (i = 1; i <= DEFAULT_STRLEN; ++i)
		printf(", \"%s\"", b[i]);
	printf("]");
#else
	printf("], %p /* %u vars */", b, DEFAULT_STRLEN + 1);
#endif
	printf(") = -1 ENOENT (%m)\n");

	execve(FILENAME, a + 1, b + 1);
	printf("execve(\"%s\", [\"%s\"", Q_FILENAME, a[1]);
	for (i = 2; i <= DEFAULT_STRLEN; ++i)
		printf(", \"%s\"", a[i]);
#if VERBOSE
	printf("], [\"%s\"", b[1]);
	for (i = 2; i <= DEFAULT_STRLEN; ++i)
		printf(", \"%s\"", b[i]);
	printf("]");
#else
	printf("], %p /* %d vars */", b + 1, DEFAULT_STRLEN);
#endif
	printf(") = -1 ENOENT (%m)\n");

	execve(FILENAME, (char **) tail_argv[ARRAY_SIZE(q_argv)], efault);
	printf("execve(\"%s\", NULL, %p) = -1 ENOENT (%m)\n",
	       Q_FILENAME, efault);

	execve(FILENAME, efault, NULL);
	printf("execve(\"%s\", %p, NULL) = -1 ENOENT (%m)\n",
	       Q_FILENAME, efault);

	return 0;
}
