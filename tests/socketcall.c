/*
 * Check decoding of socketcall syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_socketcall && !defined __ARM_EABI__

# include <assert.h>
# include <stdio.h>
# include <unistd.h>

# include "xlat.h"
# include "xlat/socketcalls.h"

static const char *
xlookup_uint(const struct xlat *xlat, const unsigned int val)
{
	for (size_t i = 0; i < xlat->size; i++)
		if (xlat->data[i].val == val)
			return xlat->data[i].str;
	return NULL;
}

static const int sc_min = 1, sc_max = 20;
static void *efault;

static void
test_socketcall(const int i, const void *const addr)
{
	const unsigned long call =
		(unsigned long) 0xfacefeed00000000ULL | (unsigned int) i;

	long rc = syscall(__NR_socketcall, call, addr);

	if (i < sc_min || i > sc_max) {
		printf("socketcall(%d, %p) = %ld %s (%m)\n",
		       (int) call, addr, rc, errno2name());
	} else if (addr == efault) {
		const char *const str = xlookup_uint(socketcalls, i);
		assert(str);
		printf("socketcall(%s, %p) = %ld %s (%m)\n",
		       str, addr, rc, errno2name());
	}
}
int
main(void)
{
	assert(0 == socketcalls->data[0].val);
	assert((unsigned) sc_max == socketcalls->data[socketcalls->size - 1].val);

	const unsigned long *const args = tail_alloc(sizeof(*args) * 6);
	efault = tail_alloc(1) + 1;

	for (int i = sc_min - 3; i <= sc_max + 3; ++i) {
		test_socketcall(i, efault);
		test_socketcall(i, args);
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_socketcall && !__ARM_EABI__")

#endif
