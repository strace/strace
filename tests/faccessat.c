/*
 * Check decoding of faccessat syscall.
 *
 * Copyright (c) 2016-2020 The strace developers.
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

# ifndef FD_PATH
#  define FD_PATH ""
# endif
# ifndef SKIP_IF_PROC_IS_UNAVAILABLE
#  define SKIP_IF_PROC_IS_UNAVAILABLE
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

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	TAIL_ALLOC_OBJECT_CONST_PTR(const char, unterminated);
	char *unterminated_str;
	if (asprintf(&unterminated_str, "%p", unterminated) < 0)
                perror_msg_and_fail("asprintf");
	const void *const efault = unterminated + 1;
	char *efault_str;
	if (asprintf(&efault_str, "%p", efault) < 0)
                perror_msg_and_fail("asprintf");

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
	char *fd_str;
	if (asprintf(&fd_str, "%d%s", fd, FD_PATH) < 0)
                perror_msg_and_fail("asprintf");
	char *path_quoted;
	if (asprintf(&path_quoted, "\"%s\"", path) < 0)
                perror_msg_and_fail("asprintf");

	struct {
		int val;
		const char *str;
	} dirfds[] = {
		{ ARG_STR(-1) },
		{ -100, "AT_FDCWD" },
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
# ifdef PATH_TRACING
				if (dirfds[dirfd_i].val == fd ||
				    paths[path_i].val == fd_path)
# endif
				printf("faccessat(%s, %s, %s) = %s\n",
				       dirfds[dirfd_i].str,
				       paths[path_i].str,
				       modes[mode_i].str,
				       errstr);
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_faccessat")

#endif
