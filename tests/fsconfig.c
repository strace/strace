/*
 * Check decoding of fsconfig syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_fsconfig

# include <fcntl.h>
# include <limits.h>
# include <stdio.h>
# include <stdint.h>
# include <unistd.h>

# define XLAT_MACROS_ONLY
# include "xlat/fsconfig_cmds.h"
# undef XLAT_MACROS_ONLY

static const char *errstr;

static long
k_fsconfig(const unsigned int fs_fd,
	   const unsigned int cmd,
	   const void *key,
	   const void *value,
	   const unsigned int aux)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | fs_fd;
	const kernel_ulong_t arg2 = fill | cmd;
	const kernel_ulong_t arg3 = (uintptr_t) key;
	const kernel_ulong_t arg4 = (uintptr_t) value;
	const kernel_ulong_t arg5 = fill | aux;
	const long rc = syscall(__NR_fsconfig,
				arg1, arg2, arg3, arg4, arg5, bad);
	errstr = sprintrc(rc);
	return rc;
}

static const char path_full[] = "/dev/full";
static const int max_string_size = 256;
static const int max_blob_size = 300;
static const int huge_blob_size = 1024 * 1024 + 1;
static const char *fd_path;
static const void *efault;
static const char *empty;
static char *fname;
static char *key1;
static char *key;
static char *val1;
static char *val;
static char *blob1;
static char *blob;
static int fd;

static void
test_fsconfig_unknown(void)
{
	k_fsconfig(fd, 8, empty, val, -100);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, 0x8 /* FSCONFIG_??? */, %p, %p, -100) = %s\n",
	       fd, fd_path, empty, val, errstr);
# endif

	k_fsconfig(-100, -1, empty, fd_path, fd);
# ifndef PATH_TRACING
	printf("fsconfig(-100, 0xffffffff /* FSCONFIG_??? */, %p, %p, %d)"
	       " = %s\n",
	       empty, fd_path, fd, errstr);
# endif
}

static void
test_fsconfig_cmd(const unsigned int cmd, const char *cmd_str)
{
	k_fsconfig(fd, cmd, empty, val, -100);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, %p, %p, -100) = %s\n",
	       fd, fd_path, cmd_str, empty, val, errstr);
# endif

	k_fsconfig(-100, cmd, empty, fd_path, fd);
# ifndef PATH_TRACING
	printf("fsconfig(-100, %s, %p, %p, %d) = %s\n",
	       cmd_str, empty, fd_path, fd, errstr);
# endif
}

static void
test_fsconfig_set_flag(const unsigned int cmd, const char *cmd_str)
{
	k_fsconfig(fd, cmd, key, val, -100);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%s\", %p, -100) = %s\n",
	       fd, fd_path, cmd_str, key, val, errstr);
# endif

	k_fsconfig(fd, cmd, key1, val, -100);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%.*s\"..., %p, -100) = %s\n",
	       fd, fd_path, cmd_str, max_string_size, key1, val, errstr);
# endif

	k_fsconfig(-100, cmd, key, fd_path, fd);
# ifndef PATH_TRACING
	printf("fsconfig(-100, %s, \"%s\", %p, %d) = %s\n",
	       cmd_str, key, fd_path, fd, errstr);
# endif
}

static void
test_fsconfig_set_string(const unsigned int cmd, const char *cmd_str)
{
	k_fsconfig(fd, cmd, key, val, -100);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%s\", \"%s\", -100) = %s\n",
	       fd, fd_path, cmd_str, key, val, errstr);
# endif

	k_fsconfig(fd, cmd, key1, val1, -100);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%.*s\"..., \"%.*s\"..., -100) = %s\n",
	       fd, fd_path, cmd_str, max_string_size, key1, max_string_size, val1,
	       errstr);
# endif

	k_fsconfig(-100, cmd, key, fd_path, fd);
# ifndef PATH_TRACING
	printf("fsconfig(-100, %s, \"%s\", \"%s\", %d) = %s\n",
	       cmd_str, key, fd_path, fd, errstr);
# endif
}

static void
test_fsconfig_set_binary(const unsigned int cmd, const char *cmd_str)
{
	k_fsconfig(fd, cmd, key, blob, max_blob_size);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%s\", ", fd, fd_path, cmd_str, key);
	print_quoted_hex(blob, max_blob_size);
	printf(", %d) = %s\n", max_blob_size, errstr);
# endif

	k_fsconfig(fd, cmd, key1, blob1, max_blob_size + 1);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%.*s\"..., ",
	       fd, fd_path, cmd_str, max_string_size, key1);
	print_quoted_hex(blob1, max_blob_size);
	printf("..., %d) = %s\n", max_blob_size + 1, errstr);
# endif

	k_fsconfig(fd, cmd, key, empty, 0);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%s\", \"\", 0) = %s\n",
	       fd, fd_path, cmd_str, key, errstr);
# endif

	k_fsconfig(fd, cmd, key, fname, -100);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%s\", %p, -100) = %s\n",
	       fd, fd_path, cmd_str, key, fname, errstr);
# endif

	k_fsconfig(fd, cmd, key, fname, huge_blob_size);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%s\", %p, %d) = %s\n",
	       fd, fd_path, cmd_str, key, fname, huge_blob_size, errstr);
# endif

	k_fsconfig(-100, cmd, key, fd_path, sizeof(path_full));
# ifndef PATH_TRACING
	printf("fsconfig(-100, %s, \"%s\", ", cmd_str, key);
	print_quoted_hex(fd_path, sizeof(path_full));
	printf(", %d) = %s\n", (int) sizeof(path_full), errstr);
# endif

	k_fsconfig(-100, cmd, key, fname, fd);
# ifndef PATH_TRACING
	printf("fsconfig(-100, %s, \"%s\", ", cmd_str, key);
	print_quoted_hex(fname, fd);
	printf(", %d) = %s\n", fd, errstr);
# endif
}

static void
test_fsconfig_set_path(const unsigned int cmd, const char *cmd_str)
{
	fill_memory_ex(fname, PATH_MAX, '0', 10);
	k_fsconfig(fd, cmd, key, fname, -100);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%s\", \"%.*s\"..., AT_FDCWD) = %s\n",
	       fd, fd_path, cmd_str, key, (int) PATH_MAX - 1, fname, errstr);
# endif

	fname[PATH_MAX - 1] = '\0';
	k_fsconfig(fd, cmd, key, fname, -1);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%s\", \"%s\", -1) = %s\n",
	       fd, fd_path, cmd_str, key, fname, errstr);
# endif

	k_fsconfig(-100, cmd, key, empty, fd);
	printf("fsconfig(-100, %s, \"%s\", \"\", %d<%s>) = %s\n",
	       cmd_str, key, fd, fd_path, errstr);

	k_fsconfig(-1, cmd, 0, fd_path, -100);
	printf("fsconfig(-1, %s, NULL, \"%s\", AT_FDCWD) = %s\n",
	       cmd_str, fd_path, errstr);

	k_fsconfig(-1, cmd, efault, efault + 1, fd);
	printf("fsconfig(-1, %s, %p, %p, %d<%s>) = %s\n",
	       cmd_str, efault, efault + 1, fd, fd_path, errstr);

	k_fsconfig(-100, cmd, key, fname, -1);
# ifndef PATH_TRACING
	printf("fsconfig(-100, %s, \"%s\", \"%s\", -1) = %s\n",
	       cmd_str, key, fname, errstr);
# endif
}

static void
test_fsconfig_set_fd(const unsigned int cmd, const char *cmd_str)
{
	k_fsconfig(fd, cmd, key, val, -100);
# ifndef PATH_TRACING
	printf("fsconfig(%d<%s>, %s, \"%s\", %p, -100) = %s\n",
	       fd, fd_path, cmd_str, key, val, errstr);
# endif

	k_fsconfig(-100, cmd, key1, 0, fd);
	printf("fsconfig(-100, %s, \"%.*s\"..., NULL, %d<%s>) = %s\n",
	       cmd_str, max_string_size, key1, fd, fd_path, errstr);

	k_fsconfig(-1, cmd, efault, efault + 1, fd);
	printf("fsconfig(-1, %s, %p, %p, %d<%s>) = %s\n",
	       cmd_str, efault, efault + 1, fd, fd_path, errstr);

	k_fsconfig(-100, cmd, key, fd_path, -1);
# ifndef PATH_TRACING
	printf("fsconfig(-100, %s, \"%s\", %p, -1) = %s\n",
	       cmd_str, key, fd_path, errstr);
# endif
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	fd_path = tail_memdup(path_full, sizeof(path_full));
	efault = fd_path + sizeof(path_full);
	empty = efault - 1;
	fname = tail_alloc(PATH_MAX);
	key1 = tail_alloc(max_string_size + 1);
	key = key1 + 1;
	val1 = tail_alloc(max_string_size + 1);
	val = val1 + 1;
	blob1 = tail_alloc(max_blob_size + 1);
	blob = blob1 + 1;

	fill_memory_ex(fname, PATH_MAX, '0', 10);
	fill_memory_ex(key1, max_string_size, 'a', 'z' - 'a' + 1);
	key1[max_string_size] = '\0';
	fill_memory_ex(val1, max_string_size, 'A', 'Z' - 'A' + 1);
	val1[max_string_size] = '\0';
	fill_memory_ex(blob, max_blob_size, '0', 10);
	blob[0] = 0;
	blob1[0] = 0;
        fd = open(fd_path, O_WRONLY);
        if (fd < 0)
                perror_msg_and_fail("open: %s", fd_path);

	test_fsconfig_unknown();
	test_fsconfig_set_flag(ARG_STR(FSCONFIG_SET_FLAG));
	test_fsconfig_set_string(ARG_STR(FSCONFIG_SET_STRING));
	test_fsconfig_set_binary(ARG_STR(FSCONFIG_SET_BINARY));
	test_fsconfig_set_path(ARG_STR(FSCONFIG_SET_PATH));
	test_fsconfig_set_path(ARG_STR(FSCONFIG_SET_PATH_EMPTY));
	test_fsconfig_set_fd(ARG_STR(FSCONFIG_SET_FD));
	test_fsconfig_cmd(ARG_STR(FSCONFIG_CMD_CREATE));
	test_fsconfig_cmd(ARG_STR(FSCONFIG_CMD_RECONFIGURE));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fsconfig")

#endif
