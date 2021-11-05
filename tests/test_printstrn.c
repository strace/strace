/*
 * Test printstrn/umoven.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "scno.h"
#include "test_ucopy.h"

static const char *errstr;

static void add_key(const char *addr, const unsigned int len)
{
	errstr = sprintrc(syscall(__NR_add_key, 0, 0, addr, len, -1));
}

static void
test_printstrn_at(char *const p, const unsigned int test_max)
{
	for (unsigned int i = 0; i <= test_max; ++i) {
		add_key(p + (test_max - i), i);
		printf("add_key(NULL, NULL, \"%.*s\", %u"
		       ", KEY_SPEC_THREAD_KEYRING) = %s\n",
		       (int) i, p + (test_max - i), i, errstr);
	}
}

static void
test_efault(const unsigned int test_max)
{
	char *p = tail_alloc(test_max);
	memset(p, '/', test_max);

	for (unsigned int i = 0; i <= test_max; ++i) {
		for (unsigned int j = 1; j <= sizeof(long); ++j) {
			add_key(p + (test_max - i), i + j);
			printf("add_key(NULL, NULL, %p, %u"
			       ", KEY_SPEC_THREAD_KEYRING) = %s\n",
			       p + (test_max - i), i + j, errstr);
		}
	}
}

static void
test_print_memory(char *const p, const unsigned int test_max)
{
	add_key(p, test_max);
	printf("add_key(NULL, NULL, ");
	print_quoted_memory(p, test_max);
	printf(", %u, KEY_SPEC_THREAD_KEYRING) = %s\n", test_max, errstr);
}

void
test_printstrn(const unsigned int test_max)
{
	/*
	 * abcdefgh|
	 * abcdefg|h
	 * abcdef|gh
	 * abcde|fgh
	 * abcd|efgh
	 * abc|defgh
	 * ab|cdefgh
	 * a|bcdefgh
	 * |abcdefgh
	 */
	const unsigned int page_size = get_page_size();
	char *p = tail_alloc(test_max + page_size);
	fill_memory_ex(p, test_max + page_size, 'a', 'z' - 'a' + 1);

	for (unsigned int i = 1; i <= sizeof(long); ++i)
		test_printstrn_at(p + i, test_max);
	for (unsigned int i = 0; i < sizeof(long); ++i)
		test_printstrn_at(p + page_size - i, test_max);
	test_efault(test_max);

	fill_memory_ex(p, test_max + page_size, 0x00, 0xFF);
	/* Test corner cases when octal quoting goes before digit */
	for (unsigned int i = 0; i < 11; ++i)
		p[2 + 3 * i] = '0' + i - 1;

	test_print_memory(p, test_max);
}
