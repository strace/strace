/*
 * Test printpath/umovestr.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "test_ucopy.h"

static void
test_printpath_at(char *const p, const unsigned int test_max)
{
	/*
	 *       /
	 *      /.
	 *     /..
	 *    /...
	 *   /../.
	 *  /../..
	 * /../../
	 */

	char *const eop = p + (test_max - 1);
	*eop = '\0';
	unsigned int i;
	for (i = 1; i < test_max; ++i) {
		const unsigned int i_1 = i - 1;
		memmove(eop - i, eop - i_1, i_1);
		eop[-1] = "/.."[i_1 % 3];
		if (chdir(eop - i))
			perror_msg_and_fail("chdir");
		printf("chdir(\"%s\") = 0\n", eop - i);
	}
}

static void
test_efault(const unsigned int test_max)
{
	char *p = tail_alloc(test_max);
	const char *const efault = p + test_max;
	memset(p, '/', test_max);

	for (; p <= efault; ++p) {
		if (p <= efault - PATH_MAX)
			continue;
		printf("chdir(%p) = %s\n", p, sprintrc(chdir(p)));
	}
}

static void
test_enametoolong(void)
{
	char *p = tail_alloc(PATH_MAX);
	memset(p, '/', PATH_MAX);

	printf("chdir(\"%.*s\"...) = %s\n",
	       PATH_MAX - 1, p, sprintrc(chdir(p)));
}

void
test_printpath(const unsigned int test_max)
{
	/*
	 * /../..|
	 * /../.|.
	 * /../|..
	 * /..|/..
	 * /.|./..
	 * /|../..
	 * |/../..
	 */
	const unsigned int page_size = get_page_size();
	char *p = tail_alloc(test_max + page_size);
	unsigned int i;
	for (i = 1; i < sizeof(long); ++i)
		test_printpath_at(p + i, test_max);
	for (i = 0; i < sizeof(long); ++i)
		test_printpath_at(p + page_size - i, test_max);
	test_efault(test_max);
	test_enametoolong();
}
