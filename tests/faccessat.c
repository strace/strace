/*
 * Check decoding of faccessat syscall.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_faccessat

# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

# include "secontext.h"
# include "xmalloc.h"

# ifdef FD_PATH
#  define YFLAG
# else
#  define FD_PATH ""
# endif
# ifndef SKIP_IF_PROC_IS_UNAVAILABLE
#  define SKIP_IF_PROC_IS_UNAVAILABLE
# endif

# ifdef YFLAG
#  define AT_FDCWD_FMT "<%s>"
#  define AT_FDCWD_ARG(arg) arg,
# else
#  define AT_FDCWD_FMT
#  define AT_FDCWD_ARG(arg)
# endif

static const char *errstr;

static long
k_faccessat(const unsigned int dirfd,
	    const void *const pathname,
	    const unsigned int mode)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;

	const kernel_ulong_t arg1 = fill | dirfd;
	const kernel_ulong_t arg2 = (uintptr_t) pathname;
	const kernel_ulong_t arg3 = fill | mode;
	const long rc = syscall(__NR_faccessat,
				arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

# ifndef PATH_TRACING
static void
tests_with_existing_file(void)
{
	/*
	 * Make sure the current workdir of the tracee
	 * is different from the current workdir of the tracer.
	 */
	create_and_enter_subdir("faccessat_subdir");

	int cwd_fd = get_dir_fd(".");
	char *cwd = get_fd_path(cwd_fd);

	char *my_secontext = SECONTEXT_PID_MY();

	k_faccessat(-1, NULL, F_OK);
	printf("%s%s(-1, NULL, F_OK) = %s\n",
	       my_secontext, "faccessat", errstr);

	static const char sample[] = "faccessat_sample";
	(void) unlink(sample);
	int fd = open(sample, O_CREAT|O_RDONLY, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");
	close(fd);
	char *sample_secontext = SECONTEXT_FILE(sample);

	/*
	 * Tests with AT_FDCWD.
	 */

	k_faccessat(-100, sample, F_OK);
	printf("%s%s(AT_FDCWD" AT_FDCWD_FMT ", \"%s\"%s, F_OK) = %s\n",
	       my_secontext, "faccessat",
	       AT_FDCWD_ARG(cwd)
	       sample, sample_secontext,
	       errstr);

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	k_faccessat(-100, sample, F_OK);
	printf("%s%s(AT_FDCWD" AT_FDCWD_FMT ", \"%s\", F_OK) = %s\n",
	       my_secontext, "faccessat",
	       AT_FDCWD_ARG(cwd)
	       sample,
	       errstr);

	/*
	 * Tests with dirfd.
	 */

	char *cwd_secontext = SECONTEXT_FILE(".");
	char *sample_realpath = xasprintf("%s/%s", cwd, sample);

	/* no file */
	k_faccessat(cwd_fd, sample, F_OK);
#  ifdef YFLAG
	printf("%s%s(%d<%s>%s, \"%s\", F_OK) = %s\n",
#  else
	printf("%s%s(%d%s, \"%s\", F_OK) = %s\n",
#  endif
	       my_secontext, "faccessat",
	       cwd_fd,
#  ifdef YFLAG
	       cwd,
#  endif
	       cwd_secontext,
	       sample,
	       errstr);

	fd = open(sample, O_CREAT|O_RDONLY, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");
	close(fd);

	k_faccessat(cwd_fd, sample, F_OK);
#  ifdef YFLAG
	printf("%s%s(%d<%s>%s, \"%s\"%s, F_OK) = %s\n",
#  else
	printf("%s%s(%d%s, \"%s\"%s, F_OK) = %s\n",
#  endif
	       my_secontext, "faccessat",
	       cwd_fd,
#  ifdef YFLAG
	       cwd,
#  endif
	       cwd_secontext,
	       sample, sample_secontext,
	       errstr);

	/* cwd_fd ignored when path is absolute */
	if (chdir("../.."))
		perror_msg_and_fail("chdir");

	k_faccessat(cwd_fd, sample_realpath, F_OK);
#  ifdef YFLAG
	printf("%s%s(%d<%s>%s, \"%s\"%s, F_OK) = %s\n",
#  else
	printf("%s%s(%d%s, \"%s\"%s, F_OK) = %s\n",
#  endif
	       my_secontext, "faccessat",
	       cwd_fd,
#  ifdef YFLAG
	       cwd,
#  endif
	       cwd_secontext,
	       sample_realpath, sample_secontext,
	       errstr);

	if (fchdir(cwd_fd))
		perror_msg_and_fail("fchdir");

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	leave_and_remove_subdir();
}
# endif

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

# ifndef TEST_SECONTEXT

	TAIL_ALLOC_OBJECT_CONST_PTR(const char, unterminated);
	char *unterminated_str = xasprintf("%p", unterminated);
	const void *const efault = unterminated + 1;
	char *efault_str = xasprintf("%p", efault);

	typedef struct {
		char sym;
		char null;
	} sym_null;

	TAIL_ALLOC_OBJECT_CONST_PTR(sym_null, dot);
	dot->sym = '.';
	dot->null = '\0';
	const char *const null = &dot->null;

	TAIL_ALLOC_OBJECT_CONST_PTR(sym_null, slash);
	slash->sym = '/';
	slash->null = '\0';

	static const char path[] = "/dev/full";
	const char *const fd_path = tail_memdup(path, sizeof(path));
        int fd = open(path, O_WRONLY);
        if (fd < 0)
                perror_msg_and_fail("open: %s", path);
	char *fd_str = xasprintf("%d%s", fd, FD_PATH);
	const char *at_fdcwd_str =
#  ifdef YFLAG
		xasprintf("AT_FDCWD<%s>", get_fd_path(get_dir_fd(".")));
#  else
		"AT_FDCWD";
#  endif
	char *path_quoted = xasprintf("\"%s\"", path);

	struct {
		int val;
		const char *str;
	} dirfds[] = {
		{ ARG_STR(-1) },
		{ -100, at_fdcwd_str },
		{ fd, fd_str },
	}, modes[] = {
		{ ARG_STR(F_OK) },
		{ ARG_STR(R_OK) },
		{ ARG_STR(W_OK) },
		{ ARG_STR(X_OK) },
		{ ARG_STR(R_OK|W_OK) },
		{ ARG_STR(R_OK|X_OK) },
		{ ARG_STR(W_OK|X_OK) },
		{ ARG_STR(R_OK|W_OK|X_OK) },
		{ 8, "0x8 /* ?_OK */" },
		{ -1, "R_OK|W_OK|X_OK|0xfffffff8" },
	};

	struct {
		const void *val;
		const char *str;
	} paths[] = {
		{ 0, "NULL" },
		{ efault, efault_str },
		{ unterminated, unterminated_str },
		{ null, "\"\"" },
		{ &dot->sym, "\".\"" },
		{ &slash->sym, "\"/\"" },
		{ fd_path, path_quoted },
	};

	for (unsigned int dirfd_i = 0;
	     dirfd_i < ARRAY_SIZE(dirfds);
	     ++dirfd_i) {
		for (unsigned int path_i = 0;
		     path_i < ARRAY_SIZE(paths);
		     ++path_i) {
			for (unsigned int mode_i = 0;
			     mode_i < ARRAY_SIZE(modes);
			     ++mode_i) {
				k_faccessat(dirfds[dirfd_i].val,
					    paths[path_i].val,
					    modes[mode_i].val);
#  ifdef PATH_TRACING
				if (dirfds[dirfd_i].val == fd ||
				    paths[path_i].val == fd_path)
#  endif
				printf("faccessat(%s, %s, %s) = %s\n",
				       dirfds[dirfd_i].str,
				       paths[path_i].str,
				       modes[mode_i].str,
				       errstr);
			}
		}
	}

# endif /* !TEST_SECONTEXT */

# ifndef PATH_TRACING
	tests_with_existing_file();
# endif

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_faccessat")

#endif
