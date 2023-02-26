/*
 * Check decoding of landlock_add_rule syscall.
 *
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2021-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <linux/landlock.h>

#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

#ifndef FD0_STR
# define FD0_STR ""
#endif
#ifndef RULESET_FD
# define RULESET_FD -1
#endif
#ifndef RULESET_FD_STR
# define RULESET_FD_STR "-1"
#endif
#ifndef PARENT_FD
# define PARENT_FD -1
#endif
#ifndef PARENT_FD_STR
# define PARENT_FD_STR "-1"
#endif

static const char *errstr;

static long
sys_landlock_add_rule(int ruleset_fd, unsigned int rule_type, void *rule_attr,
		      unsigned int flags)
{
	static const kernel_ulong_t fill =
		(kernel_ulong_t) 0xd1efaced00000000ULL;
	kernel_ulong_t arg1 = fill | (unsigned int) ruleset_fd;
	kernel_ulong_t arg2 = fill | rule_type;
	kernel_ulong_t arg3 = (uintptr_t) rule_attr;
	kernel_ulong_t arg4 = fill | flags;
	kernel_ulong_t arg5 = fill | 0xdecaffed;
	kernel_ulong_t arg6 = fill | 0xdeefaced;

	long rc = syscall(__NR_landlock_add_rule,
			  arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	TAIL_ALLOC_OBJECT_VAR_PTR(struct landlock_path_beneath_attr, attr);

	/* All zeroes */
	sys_landlock_add_rule(0, 0, NULL, 0);
	printf("landlock_add_rule(0" FD0_STR ", 0 /* LANDLOCK_RULE_??? */"
	       ", NULL, 0) = %s\n", errstr);

	/* Bogus values */
	sys_landlock_add_rule(0xdeadc0de, 0xfacebeef, attr + 1, 1);
	printf("landlock_add_rule(-559038242"
	       ", 0xfacebeef /* LANDLOCK_RULE_??? */, %p, 0x1) = %s\n",
	       attr + 1, errstr);

	sys_landlock_add_rule(1729, 2, attr + 1, 0xffffffff);
	printf("landlock_add_rule(1729, 0x2 /* LANDLOCK_RULE_??? */, %p"
	       ", 0xffffffff) = %s\n",
	       attr + 1, errstr);

	/* Invalid pointer */
	sys_landlock_add_rule(RULESET_FD, LANDLOCK_RULE_PATH_BENEATH,
			      attr + 1, 0);
	printf("landlock_add_rule(" RULESET_FD_STR
	       ", LANDLOCK_RULE_PATH_BENEATH, %p, 0) = %s\n",
	       attr + 1, errstr);

	/* Short read */
	sys_landlock_add_rule(RULESET_FD, LANDLOCK_RULE_PATH_BENEATH,
			      (char *) attr + 4, 0);
	printf("landlock_add_rule(" RULESET_FD_STR
	       ", LANDLOCK_RULE_PATH_BENEATH, %p, 0) = %s\n",
	       (char *) attr + 4, errstr);

	/* Valid attr ptr */
	static const struct {
		uint64_t val;
		const char *str;
	} attr_vals[] = {
		{ ARG_STR(LANDLOCK_ACCESS_FS_EXECUTE) },
		{ ARG_ULL_STR(LANDLOCK_ACCESS_FS_EXECUTE|LANDLOCK_ACCESS_FS_READ_FILE|LANDLOCK_ACCESS_FS_READ_DIR|LANDLOCK_ACCESS_FS_REMOVE_FILE|LANDLOCK_ACCESS_FS_MAKE_CHAR|LANDLOCK_ACCESS_FS_MAKE_DIR|LANDLOCK_ACCESS_FS_MAKE_SOCK|LANDLOCK_ACCESS_FS_MAKE_FIFO|LANDLOCK_ACCESS_FS_MAKE_BLOCK|LANDLOCK_ACCESS_FS_MAKE_SYM|LANDLOCK_ACCESS_FS_REFER|LANDLOCK_ACCESS_FS_TRUNCATE|0xdebeefeddeca8000) },
		{ ARG_ULL_STR(0xdebeefeddeca8000)
			" /* LANDLOCK_ACCESS_FS_??? */" },
	};
	static const struct {
		int val;
		const char *str;
	} parent_fd_vals[] = {
		{ ARG_STR(-1) },
		{ ARG_STR(11630) },
		{ PARENT_FD, PARENT_FD_STR },
	};
	for (size_t i = 0; i < ARRAY_SIZE(attr_vals); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(parent_fd_vals); j++) {
			attr->allowed_access = attr_vals[i].val;
			attr->parent_fd = parent_fd_vals[j].val;
			sys_landlock_add_rule(RULESET_FD,
					      LANDLOCK_RULE_PATH_BENEATH,
					      attr, 0);
			printf("landlock_add_rule(" RULESET_FD_STR
			       ", LANDLOCK_RULE_PATH_BENEATH"
			       ", {allowed_access=%s, parent_fd=%s}, 0) = %s\n",
			       attr_vals[i].str, parent_fd_vals[j].str, errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
