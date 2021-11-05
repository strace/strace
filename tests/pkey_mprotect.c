/*
 * Check decoding of pkey_mprotect syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

static const char *
sprintptr(kernel_ulong_t ptr)
{
	static char buf[sizeof(ptr) * 2 + sizeof("0x")];

	if (ptr)
		snprintf(buf, sizeof(buf), "%#llx", (unsigned long long) ptr);
	else
		return "NULL";

	return buf;
}

int
main(void)
{
	static const kernel_ulong_t ptrs[] = {
		0,
		(kernel_ulong_t) 0xfacebeef00000000ULL,
		(kernel_ulong_t) 0xbadc0dedda7a1057ULL,
	};
	static const kernel_ulong_t sizes[] = {
		0,
		(kernel_ulong_t) 0xfacebeef00000000ULL,
		(kernel_ulong_t) 0xfedcba9876543210ULL,
		(kernel_ulong_t) 0x123456789abcdef0ULL,
		(kernel_ulong_t) 0xbadc0dedda7a1057ULL,
	};
	static const struct {
		kernel_ulong_t val;
		const char *str;
	} prots[] = {
		{ ARG_STR(PROT_READ) },
		/* For now, only 0x0300001f are used */
		{ (kernel_ulong_t) 0xdeadfeed00ca7500ULL,
			sizeof(kernel_ulong_t) > sizeof(int) ?
			"0xdeadfeed00ca7500 /* PROT_??? */" :
			"0xca7500 /* PROT_??? */" },
		{ ARG_STR(PROT_READ|PROT_WRITE|0xface00) },
	};
	static const kernel_ulong_t pkeys[] = {
		0,
		-1LL,
		(kernel_ulong_t) 0xface1e55,
		(kernel_ulong_t) 0xbadc0ded00000001,
	};

	for (unsigned int i = 0;
	     i < ARRAY_SIZE(ptrs); ++i) {
		for (unsigned int j = 0;
		     j < ARRAY_SIZE(sizes); ++j) {
			for (unsigned int k = 0;
			     k < ARRAY_SIZE(prots); ++k) {
				for (unsigned int l = 0;
				     l < ARRAY_SIZE(pkeys); ++l) {
					long rc = syscall(__NR_pkey_mprotect,
							  ptrs[i], sizes[j],
							  prots[k].val, pkeys[l]);
					printf("pkey_mprotect(%s, %llu, %s, %d)"
					       " = %s\n",
					       sprintptr(ptrs[i]),
					       (unsigned long long) sizes[j],
					       prots[k].str, (int) pkeys[l],
					       sprintrc(rc));
				}
			}
		}
	}

	puts("+++ exited with 0 +++");

	return 0;
}
