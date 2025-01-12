/*
 * Check decoding of listxattrat syscall.
 *
 * Copyright (c) 2016-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include "xmalloc.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/xattr.h>

#define XLAT_MACROS_ONLY
# include "xlat/xattrat_flags.h"
#undef XLAT_MACROS_ONLY

#ifndef XATTR_LIST_MAX
# define XATTR_LIST_MAX 65536
#endif

#ifndef FD_PATH
# define FD_PATH ""
#else
# define YFLAG
#endif
#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

static const char *errstr;

static long
k_listxattrat(const unsigned int dirfd,
		const void *const pathname,
		const unsigned int flags,
		const void *const addr,
		const unsigned long size)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;

	const kernel_ulong_t arg1 = fill | dirfd;
	const kernel_ulong_t arg2 = (uintptr_t) pathname;
	const kernel_ulong_t arg3 = fill | flags;
	const kernel_ulong_t arg4 = (uintptr_t) addr;
	const kernel_ulong_t arg5 = size;
	const long rc = syscall(__NR_listxattrat,
				arg1, arg2, arg3, arg4, arg5, bad);
	errstr = sprintrc(rc);
	return rc;
}

static long
k_setxattrat(const unsigned int dirfd,
		const void *const pathname,
		const unsigned int flags,
		const void *const name,
		const void *const args,
		const unsigned long size)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;

	const kernel_ulong_t arg1 = fill | dirfd;
	const kernel_ulong_t arg2 = (uintptr_t) pathname;
	const kernel_ulong_t arg3 = fill | flags;
	const kernel_ulong_t arg4 = (uintptr_t) name;
	const kernel_ulong_t arg5 = (uintptr_t) args;
	const kernel_ulong_t arg6 = size;
	const long rc = syscall(__NR_setxattrat,
				arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

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
#ifdef YFLAG
		xasprintf("AT_FDCWD<%s>", get_fd_path(get_dir_fd(".")));
#else
		"AT_FDCWD";
#endif

	char *path_quoted = xasprintf("\"%s\"", path);

	struct strival32 dirfds[] = {
		{ ARG_STR(-1) },
		{ -100, at_fdcwd_str },
		{ fd, fd_str },
	}, flags[] = {
		{ ARG_STR(0) },
		{ ARG_STR(AT_SYMLINK_NOFOLLOW) },
		{ ARG_STR(AT_EMPTY_PATH) },
		{ ARG_STR(AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH) },
		{ 1, "0x1 /* AT_??? */" },
		{ -1, "AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH|0xffffeeff" },
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

	char *const big = tail_alloc(XATTR_LIST_MAX);

	void *addrs[] = {
		big,
		big + XATTR_LIST_MAX
	};

	const unsigned long sizes[] = {
		0,
		XATTR_LIST_MAX,
	};

	static const char c_value[] = {'f', 'o', 'o', '\0', 'b', 'a', 'r'};
	struct xattr_args args = {
		.value = (uintptr_t) c_value,
		.size = sizeof(c_value)
	};
	k_setxattrat(-100, ".", 0, "user.strace.test.listxattrat",
		     &args, sizeof(args));

	for (unsigned int dirfd_i = 0;
	     dirfd_i < ARRAY_SIZE(dirfds);
	     ++dirfd_i) {
		for (unsigned int path_i = 0;
		     path_i < ARRAY_SIZE(paths);
		     ++path_i) {
			for (unsigned int flag_i = 0;
			     flag_i < ARRAY_SIZE(flags);
			     ++flag_i) {
				for (unsigned int addr_i = 0;
				     addr_i < ARRAY_SIZE(addrs);
				     ++addr_i) {
					for (unsigned int size_i = 0;
					     size_i < ARRAY_SIZE(sizes);
					     ++size_i) {
						long rc = k_listxattrat(dirfds[dirfd_i].val,
									paths[path_i].val,
									flags[flag_i].val,
									addrs[addr_i],
									sizes[size_i]);
#ifdef PATH_TRACING
						if (dirfds[dirfd_i].val == fd ||
						    paths[path_i].val == fd_path)
#endif
						{
							printf("listxattrat(%s, %s, %s, ",
							       dirfds[dirfd_i].str,
							       paths[path_i].str,
							       flags[flag_i].str);
							if (rc < 0 || sizes[size_i] == 0)
								printf("%p", addrs[addr_i]);
							else {
								const int ellipsis =
									rc > DEFAULT_STRLEN;
								const size_t size =
									ellipsis ? DEFAULT_STRLEN : rc;

								print_quoted_memory(addrs[addr_i],
										    size);
								if (ellipsis)
									fputs("...", stdout);
							}
							printf(", %lu) = %s\n",
							       sizes[size_i], errstr);
						}
					}
				}
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
