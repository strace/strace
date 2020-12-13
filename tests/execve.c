/*
 * This file is part of execve strace test.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <unistd.h>

static const char *errstr;

static int
call_execve(const char *pathname, char *const *argv, char *const *envp)
{
	int rc = execve(pathname, argv, envp);
	errstr = sprintrc(rc);
	return rc;
}

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

	call_execve(FILENAME, tail_argv, tail_envp);
	printf("execve(\"%s\""
	       ", [\"%s\", \"%s\", \"%s\", %p, %p, %p, ... /* %p */]"
#if VERBOSE
	       ", [\"%s\", \"%s\", %p, %p, %p, ... /* %p */]"
#else
	       ", %p /* 5 vars, unterminated */"
#endif
	       ") = %s\n",
	       Q_FILENAME, q_argv[0], q_argv[1], q_argv[2],
	       argv[3], argv[4], argv[5], (char *) tail_argv + sizeof(argv)
#if VERBOSE
	       , q_envp[0], q_envp[1], envp[2], envp[3], envp[4],
	       (char *) tail_envp + sizeof(envp)
#else
	       , tail_envp
#endif
	       , errstr);

	tail_argv[ARRAY_SIZE(q_argv)] = NULL;
	tail_envp[ARRAY_SIZE(q_envp)] = NULL;
	(void) q_envp;	/* workaround for clang bug #33068 */

	call_execve(FILENAME, tail_argv, tail_envp);
	printf("execve(\"%s\", [\"%s\", \"%s\", \"%s\"]"
#if VERBOSE
	       ", [\"%s\", \"%s\"]"
#else
	       ", %p /* 2 vars */"
#endif
	       ") = %s\n",
	       Q_FILENAME, q_argv[0], q_argv[1], q_argv[2]
#if VERBOSE
	       , q_envp[0], q_envp[1]
#else
	       , tail_envp
#endif
	       , errstr);

	call_execve(FILENAME, tail_argv + 2, tail_envp + 1);
	printf("execve(\"%s\", [\"%s\"]"
#if VERBOSE
	       ", [\"%s\"]"
#else
	       ", %p /* 1 var */"
#endif
	       ") = %s\n",
	       Q_FILENAME, q_argv[2]
#if VERBOSE
	       , q_envp[1]
#else
	       , tail_envp + 1
#endif
	       , errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(char *, empty);
	char **const efault = empty + 1;
	*empty = NULL;

	call_execve(FILENAME, empty, empty);
	printf("execve(\"%s\", []"
#if VERBOSE
	       ", []"
#else
	       ", %p /* 0 vars */"
#endif
	       ") = %s\n", Q_FILENAME
#if !VERBOSE
	       , empty
#endif
	       , errstr);

	char *const str_a = tail_alloc(DEFAULT_STRLEN + 2);
	fill_memory_ex(str_a, DEFAULT_STRLEN + 1, '0', 10);
	str_a[DEFAULT_STRLEN + 1] = '\0';

	char *const str_b = tail_alloc(DEFAULT_STRLEN + 2);
	fill_memory_ex(str_b, DEFAULT_STRLEN + 1, '_', 32);
	str_b[DEFAULT_STRLEN + 1] = '\0';

	char **const a = tail_alloc(sizeof(*a) * (DEFAULT_STRLEN + 2));
	char **const b = tail_alloc(sizeof(*b) * (DEFAULT_STRLEN + 2));
	unsigned int i;
	for (i = 0; i <= DEFAULT_STRLEN; ++i) {
		a[i] = &str_a[i];
		b[i] = &str_b[i];
	}
	a[i] = b[i] = NULL;

	call_execve(FILENAME, a, b);
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
	printf(") = %s\n", errstr);

	call_execve(FILENAME, a + 1, b + 1);
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
	printf(") = %s\n", errstr);

	call_execve(FILENAME, (char **) tail_argv[ARRAY_SIZE(q_argv)], efault);
	printf("execve(\"%s\", NULL, %p) = %s\n",
	       Q_FILENAME, efault, errstr);

	call_execve(FILENAME, efault, NULL);
	printf("execve(\"%s\", %p, NULL) = %s\n",
	       Q_FILENAME, efault, errstr);

	return 0;
}
