/*
 * Check decoding of landlock_create_ruleset syscall.
 *
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2021-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "xmalloc.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/landlock.h>

#ifndef RETVAL_INJECTED
# define RETVAL_INJECTED 0
#endif
#ifndef DECODE_FD
# define DECODE_FD 0
#endif

#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif
#ifndef FD_PATH
# define FD_PATH ""
#endif

#if RETVAL_INJECTED
# define INJ_STR " (INJECTED)\n"
# define INJ_FD_STR FD_PATH " (INJECTED)\n"
#else /* !RETVAL_INJECTED */
# define INJ_STR "\n"
# define INJ_FD_STR "\n"
#endif /* RETVAL_INJECTED */

static const char *errstr;

static long
sys_landlock_create_ruleset(void *attr, kernel_ulong_t size, unsigned int flags)

{
	static const kernel_ulong_t fill =
		(kernel_ulong_t) 0xd1efaced00000000ULL;
	kernel_ulong_t arg1 = (uintptr_t) attr;
	kernel_ulong_t arg2 = size;
	kernel_ulong_t arg3 = fill | flags;
	kernel_ulong_t arg4 = fill | 0xbadbeefd;
	kernel_ulong_t arg5 = fill | 0xdecaffed;
	kernel_ulong_t arg6 = fill | 0xdeefaced;

	long rc = syscall(__NR_landlock_create_ruleset,
			  arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

static const char *
get_fd_str(const long int fd)
{
	if (fd < 0)
		return "";

#if DECODE_FD
	/*
	 * The ABI has been broken in commit v5.18-rc1~88^2
	 * by adding brackets to the link value, hence, we can't
	 * rely on a specific name anymore and have to fetch it
	 * ourselves.
	 */

	static char buf[256];
	char *path = xasprintf("/proc/self/fd/%ld", fd);
	ssize_t rc = readlink(path, buf + 1, sizeof(buf) - 3);
	free(path);

	if (rc >= 0) {
		buf[0] = '<';
		buf[rc + 1] = '>';
		buf[rc + 2] = '\0';
		return buf;
	}
#endif

	return FD_PATH;
}

int
main(void)
{
	static const kernel_ulong_t bogus_size =
		(kernel_ulong_t) 0xbadfaceddecaffee;

	SKIP_IF_PROC_IS_UNAVAILABLE;

	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, handled_access_fs);
	struct attr {
		uint64_t handled_access_fs;
		uint64_t handled_access_net;
		uint64_t scoped;
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(struct attr, attr);
	long rc;

	/* All zeroes */
	rc = sys_landlock_create_ruleset(NULL, 0, 0);
	printf("landlock_create_ruleset(NULL, 0, 0) = %s" INJ_FD_STR,
	       errstr);

	/* Get ABI version */
	rc = sys_landlock_create_ruleset(NULL, 0, 1);
	printf("landlock_create_ruleset(NULL, 0"
	       ", LANDLOCK_CREATE_RULESET_VERSION) = %s" INJ_STR, errstr);

	/* ilp32 check */
	rc = syscall(__NR_landlock_create_ruleset,
		     (kernel_ulong_t) 0xffFFffFF00000000,
		     (kernel_ulong_t) 0xdefeededdeadface,
		     (kernel_ulong_t) 0xbadc0dedbadfaced);
	printf("landlock_create_ruleset("
#if SIZEOF_KERNEL_LONG_T > 4
	       "%#llx"
#else
	       "NULL"
#endif
	       ", %llu, LANDLOCK_CREATE_RULESET_VERSION|%#x) = %s" INJ_STR,
#if SIZEOF_KERNEL_LONG_T > 4
	       (unsigned long long) (kernel_ulong_t) 0xffFFffFF00000000,
#endif
	       (unsigned long long) (kernel_ulong_t) 0xdefeededdeadface,
	       0xbadfacec, sprintrc(rc));

	/* Bogus addr, size, flags */
	rc = sys_landlock_create_ruleset(attr + 1, bogus_size,
					 0xbadcaffe);
	printf("landlock_create_ruleset(%p, %llu"
	       ", 0xbadcaffe /* LANDLOCK_CREATE_RULESET_??? */) = %s"
	       INJ_FD_STR,
	       attr + 1, (unsigned long long) bogus_size, errstr);

	/* Size is too small */
	for (size_t i = 0; i < 8; i++) {
		rc = sys_landlock_create_ruleset(handled_access_fs, i, 0);
		printf("landlock_create_ruleset(%p, %zu, 0) = %s" INJ_FD_STR,
		       handled_access_fs, i, errstr);
	}

	/* Perform syscalls with valid attr ptr */
	static const struct strval64 fs_vals[] = {
		{ ARG_STR(LANDLOCK_ACCESS_FS_EXECUTE) },
		{ ARG_ULL_STR(LANDLOCK_ACCESS_FS_EXECUTE|LANDLOCK_ACCESS_FS_READ_FILE|LANDLOCK_ACCESS_FS_READ_DIR|LANDLOCK_ACCESS_FS_REMOVE_FILE|LANDLOCK_ACCESS_FS_MAKE_CHAR|LANDLOCK_ACCESS_FS_MAKE_DIR|LANDLOCK_ACCESS_FS_MAKE_SOCK|LANDLOCK_ACCESS_FS_MAKE_FIFO|LANDLOCK_ACCESS_FS_MAKE_BLOCK|LANDLOCK_ACCESS_FS_MAKE_SYM|LANDLOCK_ACCESS_FS_REFER|LANDLOCK_ACCESS_FS_TRUNCATE|LANDLOCK_ACCESS_FS_IOCTL_DEV|0xdebeefeddeca0000) },
		{ ARG_ULL_STR(0xdebeefeddeca0000)
			" /* LANDLOCK_ACCESS_FS_??? */" },
	}, net_vals[] = {
		{ ARG_STR(LANDLOCK_ACCESS_NET_BIND_TCP) },
		{ ARG_STR(LANDLOCK_ACCESS_NET_CONNECT_TCP) },
		{ ARG_ULL_STR(LANDLOCK_ACCESS_NET_BIND_TCP|LANDLOCK_ACCESS_NET_CONNECT_TCP|0xfffffffffffffffc) },
		{ ARG_ULL_STR(0xfffffffffffffffc) " /* LANDLOCK_ACCESS_NET_??? */" },
	}, landlock_scope_flags[] = {
		{ ARG_STR(LANDLOCK_SCOPE_ABSTRACT_UNIX_SOCKET) },
		{ ARG_STR(LANDLOCK_SCOPE_SIGNAL) },
		{ ARG_ULL_STR(LANDLOCK_SCOPE_ABSTRACT_UNIX_SOCKET|LANDLOCK_SCOPE_SIGNAL|0xfffffffffffffffc) },
		{ ARG_ULL_STR(0xfffffffffffffffc) " /* LANDLOCK_SCOPE_??? */" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(fs_vals); i++) {
		unsigned int size = sizeof(*handled_access_fs);

		*handled_access_fs = fs_vals[i].val;
		rc = sys_landlock_create_ruleset(handled_access_fs, size, 0);

		printf("landlock_create_ruleset({handled_access_fs=%s}"
		       ", %u, 0) = %s%s" INJ_STR,
		       fs_vals[i].str, size,
		       errstr, get_fd_str(rc));
	}

	for (size_t i = 0; i < ARRAY_SIZE(net_vals); i++) {
		unsigned int size = offsetofend(typeof(*attr), handled_access_net);

		attr->handled_access_fs = 0;
		attr->handled_access_net = net_vals[i].val;
		rc = sys_landlock_create_ruleset(attr, size, 0);

		printf("landlock_create_ruleset({handled_access_fs=0"
		       ", handled_access_net=%s}, %u, 0) = %s%s" INJ_STR,
		       net_vals[i].str, size,
		       errstr, get_fd_str(rc));
	}

	static const unsigned int sizes[] = { sizeof(*attr), sizeof(*attr) + 4 };
	for (size_t i = 0; i < ARRAY_SIZE(landlock_scope_flags); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(sizes); j++) {
			attr->handled_access_fs = 0;
			attr->handled_access_net = 0;
			attr->scoped = landlock_scope_flags[i].val;
			rc = sys_landlock_create_ruleset(attr, sizes[j], 0);

			printf("landlock_create_ruleset({handled_access_fs=0"
			       ", handled_access_net=0, scoped=%s%s}, %u, 0)"
			       " = %s%s" INJ_STR,
			       landlock_scope_flags[i].str,
			       sizes[j] > sizeof(*attr) ? ", ..." : "",
			       sizes[j], errstr, get_fd_str(rc));
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
