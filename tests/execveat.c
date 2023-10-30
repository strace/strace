/*
 * This file is part of execveat strace test.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_execveat

# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

# include "secontext.h"

static void
tests_with_existing_file(void)
{
	/*
	 * Make sure the current workdir of the tracee
	 * is different from the current workdir of the tracer.
	 */
	create_and_enter_subdir("execveat_subdir");

	char *my_secontext = SECONTEXT_PID_MY();

	static const char sample[] = "execveat_sample";
	(void) unlink(sample);
	if (open(sample, O_RDONLY | O_CREAT, 0400) < 0)
		perror_msg_and_fail("open");

	char *sample_secontext = SECONTEXT_FILE(sample);
	static const char *argv[] = { sample, NULL };

	/*
	 * Tests with AT_FDCWD.
	 */

	long rc = syscall(__NR_execveat, -100, sample, argv, NULL, 0);
	printf("%s%s(AT_FDCWD, \"%s\"%s, [\"%s\"], NULL, 0) = %s\n",
	       my_secontext, "execveat",
	       sample, sample_secontext,
	       argv[0],
	       sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_execveat, -100, sample, argv, NULL, 0);
	printf("%s%s(AT_FDCWD, \"%s\", [\"%s\"], NULL, 0) = %s\n",
	       my_secontext, "execveat",
	       sample,
	       argv[0],
	       sprintrc(rc));

	/*
	 * Tests with dirfd.
	 */

	int cwd_fd = get_dir_fd(".");
	char *cwd = get_fd_path(cwd_fd);
	char *cwd_secontext = SECONTEXT_FILE(".");
	char *sample_realpath = xasprintf("%s/%s", cwd, sample);

	/* no file */
	rc = syscall(__NR_execveat, cwd_fd, sample, argv, NULL, 0);
	printf("%s%s(%d%s, \"%s\", [\"%s\"], NULL, 0) = %s\n",
	       my_secontext, "execveat",
	       cwd_fd, cwd_secontext,
	       sample,
	       argv[0],
	       sprintrc(rc));

	if (open(sample, O_RDONLY | O_CREAT, 0400) < 0)
		perror_msg_and_fail("open");

	rc = syscall(__NR_execveat, cwd_fd, sample, argv, NULL, 0);
	printf("%s%s(%d%s, \"%s\"%s, [\"%s\"], NULL, 0) = %s\n",
	       my_secontext, "execveat",
	       cwd_fd, cwd_secontext,
	       sample, sample_secontext,
	       argv[0],
	       sprintrc(rc));

	/* cwd_fd ignored when path is absolute */
	if (chdir("../.."))
		perror_msg_and_fail("chdir");

	rc = syscall(__NR_execveat, cwd_fd, sample_realpath, argv, NULL, 0);
	printf("%s%s(%d%s, \"%s\"%s, [\"%s\"], NULL, 0) = %s\n",
	       my_secontext, "execveat",
	       cwd_fd, cwd_secontext,
	       sample_realpath, sample_secontext,
	       argv[0],
	       sprintrc(rc));

	if (fchdir(cwd_fd))
		perror_msg_and_fail("fchdir");

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	leave_and_remove_subdir();
}

# define FILENAME "test.execveat\nfilename"
# define Q_FILENAME "test.execveat\\nfilename"

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
	const char ** const tail_argv = tail_memdup(argv, sizeof(argv));
	const char ** const tail_envp = tail_memdup(envp, sizeof(envp));
	char *my_secontext = SECONTEXT_PID_MY();

	syscall(__NR_execveat, -100, FILENAME, tail_argv, tail_envp, 0x1100);
	printf("%s%s(AT_FDCWD, \"%s\""
	       ", [\"%s\", \"%s\", \"%s\", %p, %p, %p, ... /* %p */]"
# if VERBOSE
	       ", [\"%s\", \"%s\", %p, %p, %p, ... /* %p */]"
# else
	       ", %p /* 5 vars, unterminated */"
# endif
	       ", AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH) = %s\n",
	       my_secontext, "execveat",
	       Q_FILENAME, q_argv[0], q_argv[1], q_argv[2],
	       argv[3], argv[4], argv[5], (char *) tail_argv + sizeof(argv),
# if VERBOSE
	       q_envp[0], q_envp[1], envp[2], envp[3], envp[4],
	       (char *) tail_envp + sizeof(envp),
# else
	       tail_envp,
# endif
	       sprintrc(-1));

	tail_argv[ARRAY_SIZE(q_argv)] = NULL;
	tail_envp[ARRAY_SIZE(q_envp)] = NULL;
	(void) q_envp;	/* workaround for clang bug #33068 */

	syscall(__NR_execveat, -100, FILENAME, tail_argv, tail_envp, 0x1100);
	printf("%s%s(AT_FDCWD, \"%s\", [\"%s\", \"%s\", \"%s\"]"
# if VERBOSE
	       ", [\"%s\", \"%s\"]"
# else
	       ", %p /* 2 vars */"
# endif
	       ", AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH) = %s\n",
	       my_secontext, "execveat",
	       Q_FILENAME, q_argv[0], q_argv[1], q_argv[2],
# if VERBOSE
	       q_envp[0], q_envp[1],
# else
	       tail_envp,
# endif
	       sprintrc(-1));

	syscall(__NR_execveat, -100, FILENAME, tail_argv + 2, tail_envp + 1, 0x1100);
	printf("%s%s(AT_FDCWD, \"%s\", [\"%s\"]"
# if VERBOSE
	       ", [\"%s\"]"
# else
	       ", %p /* 1 var */"
# endif
	       ", AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH) = %s\n",
	       my_secontext, "execveat",
	       Q_FILENAME, q_argv[2],
# if VERBOSE
	       q_envp[1],
# else
	       tail_envp + 1,
# endif
	       sprintrc(-1));

	TAIL_ALLOC_OBJECT_CONST_PTR(char *, empty);
	char **const efault = empty + 1;
	*empty = NULL;

	syscall(__NR_execveat, -100, FILENAME, empty, empty, 0x1100);
	printf("%s%s(AT_FDCWD, \"%s\", []"
# if VERBOSE
	       ", []"
# else
	       ", %p /* 0 vars */"
# endif
	       ", AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH) = %s\n",
	       my_secontext, "execveat",
	       Q_FILENAME,
# if !VERBOSE
	       empty,
# endif
	       sprintrc(-1));

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

	syscall(__NR_execveat, -100, FILENAME, a, b, 0x1100);
	printf("%s%s(AT_FDCWD, \"%s\", [\"%.*s\"...",
	       my_secontext, "execveat",
	       Q_FILENAME, DEFAULT_STRLEN, a[0]);
	for (i = 1; i < DEFAULT_STRLEN; ++i)
		printf(", \"%s\"", a[i]);
# if VERBOSE
	printf(", \"%s\"", a[i]);
# else
	printf(", ...");
# endif
# if VERBOSE
	printf("], [\"%.*s\"...", DEFAULT_STRLEN, b[0]);
	for (i = 1; i <= DEFAULT_STRLEN; ++i)
		printf(", \"%s\"", b[i]);
	printf("]");
# else
	printf("], %p /* %u vars */", b, DEFAULT_STRLEN + 1);
# endif
	printf(", AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH) = %s\n",
	       sprintrc(-1));

	syscall(__NR_execveat, -100, FILENAME, a + 1, b + 1, 0x1100);
	printf("%s%s(AT_FDCWD, \"%s\", [\"%s\"",
	       my_secontext, "execveat",
	       Q_FILENAME, a[1]);
	for (i = 2; i <= DEFAULT_STRLEN; ++i)
		printf(", \"%s\"", a[i]);
# if VERBOSE
	printf("], [\"%s\"", b[1]);
	for (i = 2; i <= DEFAULT_STRLEN; ++i)
		printf(", \"%s\"", b[i]);
	printf("]");
# else
	printf("], %p /* %d vars */", b + 1, DEFAULT_STRLEN);
# endif
	printf(", AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH) = %s\n",
	       sprintrc(-1));

	syscall(__NR_execveat, -100, FILENAME, NULL, efault, 0x1100);
	printf("%s%s(AT_FDCWD, \"%s\", NULL, %p"
	       ", AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH) = %s\n",
	       my_secontext, "execveat",
	       Q_FILENAME, efault, sprintrc(-1));

	syscall(__NR_execveat, -100, FILENAME, efault, NULL, 0x1100);
	printf("%s%s(AT_FDCWD, \"%s\", %p, NULL"
	       ", AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH) = %s\n",
	       my_secontext, "execveat",
	       Q_FILENAME, efault, sprintrc(-1));

	tests_with_existing_file();

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_execveat")

#endif
