/*
 * Check decoding of lsm_list_modules syscall.
 *
 * Copyright (c) 2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/lsm.h>

#ifdef INJECT_RETVAL
# define INJ_STR " (INJECTED)\n"
#else
# define INJ_STR "\n"
#endif

static const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
static const char *errstr;

static long
k_lsm_list_modules(const void *p_ids, const void *p_size, const uint32_t flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = (uintptr_t) p_ids;
	const kernel_ulong_t arg2 = (uintptr_t) p_size;
	const kernel_ulong_t arg3 = fill | flags;
	const long rc = syscall(__NR_lsm_list_modules,
				arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	k_lsm_list_modules(0, 0, 0);
	printf("lsm_list_modules(NULL, NULL, 0) = %s" INJ_STR, errstr);

	static const struct strval64 test_ids[] = {
		{ ARG_STR(LSM_ID_APPARMOR) },
		{ ARG_STR(LSM_ID_BPF) },
		{ ARG_STR(LSM_ID_CAPABILITY) },
		{ ARG_STR(LSM_ID_LANDLOCK) },
		{ ARG_STR(LSM_ID_LOADPIN) },
		{ ARG_STR(LSM_ID_LOCKDOWN) },
		{ ARG_STR(LSM_ID_SAFESETID) },
		{ ARG_STR(LSM_ID_SELINUX) },
		{ ARG_STR(LSM_ID_SMACK) },
		{ ARG_STR(LSM_ID_TOMOYO) },
		{ ARG_STR(LSM_ID_UNDEF) },
		{ ARG_STR(LSM_ID_YAMA) },
		{ 1, "0x1 /* LSM_ID_??? */" },
		{ 99, "0x63 /* LSM_ID_??? */" },
		{ 111, "0x6f /* LSM_ID_??? */" }
	};
	TAIL_ALLOC_OBJECT_CONST_ARR(uint64_t, ids, ARRAY_SIZE(test_ids));
	for (unsigned int i = 0; i < ARRAY_SIZE(test_ids); ++i)
		ids[i] = test_ids[i].val;
	void *bad_ids;

	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, size);
	const void *const bad_size = (void *) size + 1;

	k_lsm_list_modules(ids, bad_size, 1);
	printf("lsm_list_modules(%p, %p, %#x) = %s" INJ_STR,
	       ids, bad_size, 1, errstr);

	*size = sizeof(*ids) - 1;
	bad_ids = (void *) (ids + ARRAY_SIZE(test_ids) - 1) + 1;
	k_lsm_list_modules(bad_ids, size, -1);
	printf("lsm_list_modules(%p, [%u], %#x) = %s" INJ_STR,
	       bad_ids, *size, -1, errstr);

	for (unsigned int i = 0; i < ARRAY_SIZE(test_ids); ++i) {
		uint64_t *ret_ids = ids + i;
		if (k_lsm_list_modules(ret_ids, size, -1) < 0)
			printf("lsm_list_modules(%p, [%u], %#x) = %s" INJ_STR,
			       ret_ids, *size, -1, errstr);
		else {
			printf("lsm_list_modules([");
			for (unsigned int j = 0;
			     j + i < ARRAY_SIZE(test_ids);
			     ++j)
				printf("%s%s", j ? ", " : "",
				       test_ids[i + j].str);
			if (i)
				printf(", ... /* %p */",
				       ids + ARRAY_SIZE(test_ids));
			printf("], [%u], %#x) = %s" INJ_STR,
			       *size, -1, errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
