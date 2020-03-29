/*
 * Check decoding of spu_create syscall.
 *
 * Copyright (c) 2022 Eugene Syromyatnikov <evgsyr@gmail.com>.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_spu_create

# include <fcntl.h>
# include <inttypes.h>
# include <stdint.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

# define ARR_ITEM(arr_, idx_) ((arr_)[(idx_) % ARRAY_SIZE(arr_)])

# define FD0_PATH "/dev/null"
# define FD7_PATH "/dev/full"

# define PATH2_STR "0123456789abcdefghijklmnopqrstuvwxyz"

# ifndef FD0_STR
#  define FD0_STR ""
# endif

# ifndef FD7_STR
#  define FD7_STR ""
# endif

# ifndef RVAL_STR
#  define RVAL_STR ""
# endif

# ifndef PATH_FILTER
#  define PATH_FILTER 0
# endif

struct valstrflag {
	unsigned int val;
	const char *str;
	bool flag;
};

enum { CLOSED_FD = 23 };

static const char *errstr;

static long
sys_spu_create(const void *const pathname, const unsigned int flags,
	       const unsigned short mode, const int neighbor_fd)
{
	static const kernel_ulong_t fill =
		(kernel_ulong_t) 0xbeefface00000000ULL;
	kernel_ulong_t arg1 = (uintptr_t) pathname;
	kernel_ulong_t arg2 = fill | flags;
	kernel_ulong_t arg3 = fill | 0xdead0000U | mode;
	kernel_ulong_t arg4 = fill | (unsigned int) neighbor_fd;
	kernel_ulong_t arg5 = fill | 0xdecaffed;
	kernel_ulong_t arg6 = fill | 0xdeefaced;

	long rc = syscall(__NR_spu_create, arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	close(CLOSED_FD);

	/* NULL/zero args */
	sys_spu_create(NULL, 0, 0, 0);
	if (!PATH_FILTER) {
		printf("spu_create(NULL, 0, 000) = %s" RVAL_STR " (INJECTED)\n",
		       errstr);
	}

	static const struct valstrflag flags[] = {
		{ ARG_STR(0) },
		{ ARG_STR(0x1) NRAW(" /* SPU_CREATE_EVENTS_ENABLED */") },
		{ ARG_STR(0x10) NRAW(" /* SPU_CREATE_AFFINITY_SPU */"), true },
		{ ARG_STR(0x40) NRAW(" /* SPU_CREATE_??? */") },
		{ ARG_STR(0xbeef) NRAW(" /* SPU_CREATE_EVENTS_ENABLED"
				       "|SPU_CREATE_GANG|SPU_CREATE_NOSCHED"
				       "|SPU_CREATE_ISOLATE"
				       "|SPU_CREATE_AFFINITY_MEM|0xbec0 */") },
		{ ARG_STR(0xcafe) NRAW(" /* SPU_CREATE_GANG|SPU_CREATE_NOSCHED"
				       "|SPU_CREATE_ISOLATE"
				       "|SPU_CREATE_AFFINITY_SPU"
				       "|SPU_CREATE_AFFINITY_MEM|0xcac0 */"),
		  true },
		{ ARG_STR(0xfacedec0) NRAW(" /* SPU_CREATE_??? */") },
	};
	static const struct strval16 modes[] = {
		{ ARG_STR(000) },
		{ ARG_STR(0755) },
		{ ARG_STR(04000) },
	};
	static const struct valstrflag fds[] = {
		{ 0, FD0_STR },
		{ 7, FD7_STR, true },
		{ CLOSED_FD, "" },
		{ 0xdeadbeef, "" },
	};

	TAIL_ALLOC_OBJECT_VAR_ARR(char, path0, 1);
	TAIL_ALLOC_OBJECT_VAR_ARR(char, path1, 3);
	TAIL_ALLOC_OBJECT_VAR_ARR(char, path2, 37);
	TAIL_ALLOC_OBJECT_VAR_ARR(char, path3, sizeof(FD7_PATH));

	path0[0] = '\0';
	memcpy(path1, "\r\n\t", 3);
	memcpy(path2, PATH2_STR, 37);
	strcpy(path3, FD7_PATH);

	const struct {
		const char *val;
		const char *str;
		bool flag;
	} paths[] = {
		{ path0 + 1, NULL },
		{ path0, "\"\"" },
		{ path1, NULL },
		{ path2, "\"" PATH2_STR "\"" },
		{ path3, "\"" FD7_PATH "\"", true },
	};
	const size_t iters = 4 * MAX(MAX(MAX(ARRAY_SIZE(flags), ARRAY_SIZE(fds)),
					 ARRAY_SIZE(modes)), ARRAY_SIZE(paths));

	for (size_t i = 0; i < iters; i++) {
		sys_spu_create(ARR_ITEM(paths, i).val, ARR_ITEM(flags, i).val,
			       ARR_ITEM(modes, i).val, ARR_ITEM(fds, i).val);

		if (PATH_FILTER && !ARR_ITEM(paths, i).flag
		    && (!ARR_ITEM(flags, i).flag || !ARR_ITEM(fds, i).flag))
			continue;

		printf("spu_create(");
		if (ARR_ITEM(paths, i).str)
			printf("%s", ARR_ITEM(paths, i).str);
		else
			printf("%p", ARR_ITEM(paths, i).val);
		printf(", %s, %s",
		       ARR_ITEM(flags, i).str, ARR_ITEM(modes, i).str);
		if (ARR_ITEM(flags, i).flag) {
			printf(", %d%s",
			       ARR_ITEM(fds, i).val, ARR_ITEM(fds, i).str);
		}
		printf(") = %s" RVAL_STR " (INJECTED)\n", errstr);
	}

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_spu_create")

#endif
