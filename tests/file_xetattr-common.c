/*
 * Check decoding of file_getattr and file_setattr syscalls.
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
#include <string.h>
#include <unistd.h>
#include <linux/fs.h>

#include "test_fs_xflags.h"

#ifndef AT_SYMLINK_NOFOLLOW
# define AT_SYMLINK_NOFOLLOW	0x100
#endif
#ifndef AT_EMPTY_PATH
# define AT_EMPTY_PATH		0x1000
#endif

#ifndef FD_PATH
# define FD_PATH	""
#else
# define YFLAG
#endif

#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

#ifndef RETVAL_INJECTED
# define RETVAL_INJECTED	0
#endif

#if RETVAL_INJECTED
# define INJ_STR	" (INJECTED)"
#else
# define INJ_STR	""
#endif

static char errstr[1024];

static long
file_xetattr(const unsigned int dirfd,
	     const void *const pathname,
	     const void *const attr,
	     const unsigned long size,
	     const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | dirfd;
	const kernel_ulong_t arg2 = (uintptr_t) pathname;
	const kernel_ulong_t arg3 = (uintptr_t) attr;
	const kernel_ulong_t arg4 = size;
	const kernel_ulong_t arg5 = fill | flags;
	const long rc = syscall(SYSCALL_NR,
				arg1, arg2, arg3, arg4, arg5, bad);
	snprintf(errstr, sizeof(errstr), "%s%s", sprintrc(rc), INJ_STR);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct file_attr, fa);
	const unsigned int fa_size = sizeof(*fa);
	memset(fa, 0, fa_size);

	const unsigned int fa_extra_size = fa_size + 8;
	struct file_attr *const fa_extra = tail_alloc(fa_extra_size);
	memset(fa_extra, 0, fa_size);
	fill_memory(fa_extra + 1, fa_extra_size - fa_size);

	const void *const fa_short =
		(void *) (fa + 1) - (FILE_ATTR_SIZE_VER0 - 1);

	const unsigned long page_size = get_page_size();
#ifndef PATH_TRACING
	long rc;
#endif

	/* size < FILE_ATTR_SIZE_VER0, no read */
	file_xetattr(-1, "", fa, FILE_ATTR_SIZE_VER0 - 1, 0);
#ifndef PATH_TRACING
	printf("%s(-1, \"\", %p, %u, 0) = %s\n",
	       SYSCALL_NAME, fa, FILE_ATTR_SIZE_VER0 - 1, errstr);
#endif

	/* size == FILE_ATTR_SIZE_VER0, no read */
	file_xetattr(-1, "", 0, FILE_ATTR_SIZE_VER0, 0);
#ifndef PATH_TRACING
	printf("%s(-1, \"\", NULL, %u, 0) = %s\n",
	       SYSCALL_NAME, FILE_ATTR_SIZE_VER0, errstr);
#endif

	/* size == FILE_ATTR_SIZE_VER0, short read */
	file_xetattr(-1, "", fa_short, FILE_ATTR_SIZE_VER0, 0);
#ifndef PATH_TRACING
	printf("%s(-1, \"\", %p, %u, 0) = %s\n",
	       SYSCALL_NAME, fa_short, FILE_ATTR_SIZE_VER0, errstr);
#endif

	/* size > PAGE_SIZE, no read */
	file_xetattr(-1, "", fa_extra, page_size + 1, 0);
#ifndef PATH_TRACING
	printf("%s(-1, \"\", %p, %lu, 0) = %s\n",
	       SYSCALL_NAME, fa_extra, page_size + 1, errstr);
#endif

	/* size > sizeof(struct file_attr), short read */
#ifndef PATH_TRACING
	rc =
#endif
	file_xetattr(-1, "", fa, fa_extra_size, 0);
#ifndef PATH_TRACING
	if (SYSCALL_is_set || rc >= 0) {
		printf("%s(-1, \"\", {fa_xflags=0, fa_extsize=0"
		       "%s, fa_projid=0, fa_cowextsize=0, ???}"
		       ", %u, 0) = %s\n",
		       SYSCALL_NAME,
		       SYSCALL_is_set ? "" : ", fa_nextents=0",
		       fa_extra_size, errstr);
	} else {
		printf("%s(-1, \"\", %p, %u, 0) = %s\n",
		       SYSCALL_NAME, fa, fa_extra_size, errstr);
	}
#endif

	/* size > sizeof(struct file_attr), normal read */
#ifndef PATH_TRACING
	rc =
#endif
	file_xetattr(-1, "", fa_extra, fa_extra_size, 0);
#ifndef PATH_TRACING
	if (SYSCALL_is_set || rc >= 0) {
		printf("%s(-1, \"\", {fa_xflags=0, fa_extsize=0"
		       "%s, fa_projid=0, fa_cowextsize=0"
		       ", /* bytes %u..%u */ \"%s\"}"
		       ", %u, 0) = %s\n",
		       SYSCALL_NAME,
		       SYSCALL_is_set ? "" : ", fa_nextents=0",
		       fa_size, fa_extra_size - 1,
		       "\\x80\\x81\\x82\\x83\\x84\\x85\\x86\\x87",
		       fa_extra_size, errstr);
	} else {
		printf("%s(-1, \"\", %p, %u, 0) = %s\n",
		       SYSCALL_NAME, fa_extra, fa_extra_size, errstr);
	}
#endif

	/* size == sizeof(struct file_attr), normal read */
	struct strval64 xflags[] = {
		{ ARG_XLAT_KNOWN(VALID_FS_XFLAGS, VALID_FS_XFLAGS_STR) },
		{ ARG_XLAT_UNKNOWN(INVALID_FS_XFLAGS64, INVALID_FS_XFLAGS_STR) },
	};
	for (unsigned int i = 0; i < ARRAY_SIZE(xflags); ++i) {
		fa->fa_xflags = xflags[i].val;
		fa->fa_extsize = 0xdeadbeefU;
		fa->fa_nextents = 0xfacefeedU;
		fa->fa_projid = 0xcafef00dU;
		fa->fa_cowextsize = 0xbabec0deU;

#ifndef PATH_TRACING
		rc =
#endif
		file_xetattr(-1, "", fa, fa_size, 0);
#ifndef PATH_TRACING
		if (SYSCALL_is_set || rc >= 0) {
			printf("%s(-1, \"\", {fa_xflags=%s, fa_extsize=%u",
			       SYSCALL_NAME, xflags[i].str, fa->fa_extsize);
			if (!SYSCALL_is_set)
				printf(", fa_nextents=%u", fa->fa_nextents);
			printf(", fa_projid=%#x, fa_cowextsize=%u}, %u, 0) = %s\n",
			       fa->fa_projid, fa->fa_cowextsize,
			       fa_size, errstr);
		} else {
			printf("%s(-1, \"\", %p, %u, 0) = %s\n",
			       SYSCALL_NAME, fa, fa_size, errstr);
		}
#endif
	}

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
		xasprintf("%s<%s>", XLAT_KNOWN(-100, "AT_FDCWD"),
				    get_fd_path(get_dir_fd(".")));
#else
		XLAT_KNOWN(-100, "AT_FDCWD");
#endif

	char *path_quoted = xasprintf("\"%s\"", path);

	struct strival32 dirfds[] = {
		{ ARG_STR(-1) },
		{ -100, at_fdcwd_str },
		{ fd, fd_str },
	}, at_flags[] = {
		{ ARG_XLAT_KNOWN(0x100, "AT_SYMLINK_NOFOLLOW") },
		{ ARG_XLAT_KNOWN(0x1000, "AT_EMPTY_PATH") },
		{ ARG_XLAT_KNOWN(0x1100, "AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH") },
		{ ARG_XLAT_UNKNOWN(0xffffeeff, "AT_???") },
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
				file_xetattr(dirfds[dirfd_i].val,
					     paths[path_i].val,
					     0, fa_size,
					     at_flags[at_flag_i].val);
#ifdef PATH_TRACING
				if (dirfds[dirfd_i].val == fd ||
				    paths[path_i].val == fd_path)
#endif
				printf("%s(%s, %s, NULL, %u, %s) = %s\n",
				       SYSCALL_NAME,
				       dirfds[dirfd_i].str,
				       paths[path_i].str,
				       fa_size,
				       at_flags[at_flag_i].str,
				       errstr);
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
