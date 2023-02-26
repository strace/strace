/*
 * Check decoding of landlock_create_ruleset syscall.
 *
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2021-2023 The strace developers.
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
sys_landlock_create_ruleset(struct landlock_ruleset_attr *attr,
			    kernel_ulong_t size, unsigned int flags)

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

int
main(void)
{
	static const kernel_ulong_t bogus_size =
		(kernel_ulong_t) 0xbadfaceddecaffee;

	SKIP_IF_PROC_IS_UNAVAILABLE;

	TAIL_ALLOC_OBJECT_VAR_PTR(struct landlock_ruleset_attr, attr);
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
	rc = sys_landlock_create_ruleset(attr + 1, bogus_size, 0xbadcaffe);
	printf("landlock_create_ruleset(%p, %llu"
	       ", 0xbadcaffe /* LANDLOCK_CREATE_RULESET_??? */) = %s"
	       INJ_FD_STR,
	       attr + 1, (unsigned long long) bogus_size, errstr);

	/* Size is too small */
	for (size_t i = 0; i < 8; i++) {
		rc = sys_landlock_create_ruleset(attr, i, 0);
		printf("landlock_create_ruleset(%p, %zu, 0) = %s" INJ_FD_STR,
		       attr, i, errstr);
	}

	/* Perform syscalls with valid attr ptr */
	static const struct {
		uint64_t val;
		const char *str;
	} attr_vals[] = {
		{ ARG_STR(LANDLOCK_ACCESS_FS_EXECUTE) },
		{ ARG_ULL_STR(LANDLOCK_ACCESS_FS_EXECUTE|LANDLOCK_ACCESS_FS_READ_FILE|LANDLOCK_ACCESS_FS_READ_DIR|LANDLOCK_ACCESS_FS_REMOVE_FILE|LANDLOCK_ACCESS_FS_MAKE_CHAR|LANDLOCK_ACCESS_FS_MAKE_DIR|LANDLOCK_ACCESS_FS_MAKE_SOCK|LANDLOCK_ACCESS_FS_MAKE_FIFO|LANDLOCK_ACCESS_FS_MAKE_BLOCK|LANDLOCK_ACCESS_FS_MAKE_SYM|LANDLOCK_ACCESS_FS_REFER|LANDLOCK_ACCESS_FS_TRUNCATE|0xdebeefeddeca8000) },
		{ ARG_ULL_STR(0xdebeefeddeca8000)
			" /* LANDLOCK_ACCESS_FS_??? */" },
	};
	static const kernel_ulong_t sizes[] = { 8, 12, 16 };
	for (size_t i = 0; i < ARRAY_SIZE(attr_vals); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(sizes); j++) {
			const char *fd_str = FD_PATH;

			attr->handled_access_fs = attr_vals[i].val;
			rc = sys_landlock_create_ruleset(attr, sizes[j], 0);

#if DECODE_FD
			/*
			 * The ABI has been broken in commit v5.18-rc1~88^2
			 * by adding brackets to the link value, hence, we can't
			 * rely on a specific name anymore and have to fetch it
			 * ourselves.
			 */
			if (rc >= 0) {
				static char buf[256];
				char *path = xasprintf("/proc/self/fd/%ld", rc);
				ssize_t ret = readlink(path, buf + 1,
						       sizeof(buf) - 3);
				free(path);

				if (ret >= 0) {
					buf[0] = '<';
					buf[ret + 1] = '>';
					buf[ret + 2] = '\0';
					fd_str = buf;
				}
			}
#endif

			printf("landlock_create_ruleset({handled_access_fs=%s"
			       "%s}, %llu, 0) = %s%s" INJ_STR,
			       attr_vals[i].str,
			       sizes[j] > sizeof(*attr) ? ", ..." : "",
			       (unsigned long long) sizes[j],
			       errstr, rc >= 0 ? fd_str : "");
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
