/*
 * Check decoding of setxattrat syscall.
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

#ifndef XATTR_SIZE_MAX
# define XATTR_SIZE_MAX 65536
#endif

#ifndef XATTR_ARGS_SIZE_VER0
# define XATTR_ARGS_SIZE_VER0 16
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

	static const char name[] = "user.strace.test.setxattrat";
	static const char name_str[] = "\"user.strace.test.setxattrat\"";

	TAIL_ALLOC_OBJECT_CONST_PTR(struct xattr_args, args);
	const unsigned int args_size = sizeof(*args);

	const unsigned int extra_args_size = args_size + 8;
	struct xattr_args *const extra_args = tail_alloc(extra_args_size);
	fill_memory(extra_args, extra_args_size);

	const void *const short_args =
		(void *) (args + 1) - (XATTR_ARGS_SIZE_VER0 - 1);

	/* size < XATTR_ARGS_SIZE_VER0 */
	k_setxattrat(-1, "", 0, name, args, XATTR_ARGS_SIZE_VER0 - 1);
#ifndef PATH_TRACING
	printf("setxattrat(-1, \"\", 0, %s, %p, %u) = %s\n",
	       name_str, args, args_size - 1, errstr);
#endif

	/* short read of struct xattr_args */
	k_setxattrat(-1, "", 0, name, short_args, XATTR_ARGS_SIZE_VER0);
#ifndef PATH_TRACING
	printf("setxattrat(-1, \"\", 0, %s, %p, %u) = %s\n",
	       name_str, short_args, XATTR_ARGS_SIZE_VER0, errstr);
#endif

	/* size > sizeof(struct xattr_args) */
	extra_args->value = 0;
	extra_args->size = 1;
	extra_args->flags = 4;
	k_setxattrat(-1, "", 0, name, extra_args, extra_args_size);
#ifndef PATH_TRACING
	printf("setxattrat(-1, \"\", 0, %s, {value=NULL, size=1"
	       ", flags=0x4 /* XATTR_??? */, /* bytes %u..%u */ \"%s\"}"
	       ", %u) = %s\n",
	       name_str, args_size, extra_args_size - 1,
	       "\\x90\\x91\\x92\\x93\\x94\\x95\\x96\\x97",
	       extra_args_size, errstr);
#endif

	const unsigned int def_val_size = DEFAULT_STRLEN;
	const unsigned int big_val_size = def_val_size + 1;
	char *const big_val = tail_alloc(big_val_size);
	char *const def_val = big_val + 1;
	fill_memory(big_val, big_val_size);

	const unsigned int max_val_size = XATTR_SIZE_MAX;
	const unsigned int huge_val_size = max_val_size + 1;
	char *const huge_val = tail_alloc(huge_val_size);
	fill_memory(huge_val, huge_val_size);

	/* size > sizeof(struct xattr_args), short read */
	args->value = (uintptr_t) huge_val;
	args->size = huge_val_size;
	args->flags = 0;
	k_setxattrat(-1, "", 0, name, args, args_size + 1);
#ifndef PATH_TRACING
	printf("setxattrat(-1, \"\", 0, %s"
	       ", {value=%p, size=%u, flags=0, ???}, %u) = %s\n",
	       name_str, huge_val, huge_val_size,
	       args_size + 1, errstr);
#endif

	struct strival32 xattr_flags[] = {
		{ ARG_STR(0) },
		{ ARG_STR(XATTR_CREATE) },
		{ ARG_STR(XATTR_REPLACE) },
		{ ARG_STR(XATTR_CREATE|XATTR_REPLACE) },
		{ 0xfffffffc, "0xfffffffc /* XATTR_??? */" },
		{ -1, "XATTR_CREATE|XATTR_REPLACE|0xfffffffc" },
	};

	/* size == sizeof(struct xattr_args) */
	for (unsigned int xattr_flag_i = 0;
	     xattr_flag_i < ARRAY_SIZE(xattr_flags); ++xattr_flag_i) {
		args->flags = xattr_flags[xattr_flag_i].val;
		k_setxattrat(-1, "", 0, name, args, args_size);
#ifndef PATH_TRACING
		printf("setxattrat(-1, \"\", 0, %s"
		       ", {value=%p, size=%u, flags=%s}, %u) = %s\n",
		       name_str, huge_val, huge_val_size,
		       xattr_flags[xattr_flag_i].str,
		       args_size, errstr);
#endif
	}

	args->value = (uintptr_t) def_val;
	args->size = def_val_size;
	args->flags = 0;
	k_setxattrat(-1, "", 0, name, args, args_size);
#ifndef PATH_TRACING
	printf("setxattrat(-1, \"\", 0, %s, {value=", name_str);
	print_quoted_memory(def_val, def_val_size);
	printf(", size=%u, flags=0}, %u) = %s\n",
	       def_val_size, args_size, errstr);
#endif

	args->value = (uintptr_t) big_val;
	args->size = big_val_size;
	k_setxattrat(-1, "", 0, name, args, args_size);
#ifndef PATH_TRACING
	printf("setxattrat(-1, \"\", 0, %s, {value=", name_str);
	print_quoted_memory(big_val, def_val_size);
	fputs("...", stdout);
	printf(", size=%u, flags=0}, %u) = %s\n",
	       big_val_size, args_size, errstr);
#endif

	args->size = max_val_size;
	k_setxattrat(-1, "", 0, name, args, args_size);
#ifndef PATH_TRACING
	printf("setxattrat(-1, \"\", 0, %s, {value=", name_str);
	print_quoted_memory(big_val, def_val_size);
	fputs("...", stdout);
	printf(", size=%u, flags=0}, %u) = %s\n",
	       max_val_size, args_size, errstr);
#endif

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
	}, at_flags[] = {
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

	for (unsigned int dirfd_i = 0;
	     dirfd_i < ARRAY_SIZE(dirfds); ++dirfd_i) {
		for (unsigned int path_i = 0;
		     path_i < ARRAY_SIZE(paths); ++path_i) {
			for (unsigned int at_flag_i = 0;
			     at_flag_i < ARRAY_SIZE(at_flags); ++at_flag_i) {
				k_setxattrat(dirfds[dirfd_i].val,
					    paths[path_i].val,
					    at_flags[at_flag_i].val,
					    name, 0, 1);
#ifdef PATH_TRACING
				if (dirfds[dirfd_i].val == fd ||
				    paths[path_i].val == fd_path)
#endif
				printf("setxattrat(%s, %s, %s, %s, NULL, 1)"
				       " = %s\n",
				       dirfds[dirfd_i].str,
				       paths[path_i].str,
				       at_flags[at_flag_i].str,
				       name_str,
				       errstr);
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
