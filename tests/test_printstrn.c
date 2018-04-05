/*
 * Test printstrn/umoven.
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <asm/unistd.h>

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
	unsigned int i;

	for (i = 0; i <= test_max; ++i) {
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
	unsigned int i;

	for (i = 0; i <= test_max; ++i) {
		unsigned int j;
		for (j = 1; j <= sizeof(long); ++j) {
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

	unsigned int i;
	for (i = 1; i <= sizeof(long); ++i)
		test_printstrn_at(p + i, test_max);
	for (i = 0; i < sizeof(long); ++i)
		test_printstrn_at(p + page_size - i, test_max);
	test_efault(test_max);

	fill_memory_ex(p, test_max + page_size, 0x00, 0xFF);
	/* Test corner cases when octal quoting goes before digit */
	for (i = 0; i < 11; i++)
		p[2 + 3 * i] = '0' + i - 1;

	test_print_memory(p, test_max);
}
