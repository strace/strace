/*
 * Test printpath/umovestr.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
