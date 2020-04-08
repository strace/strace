/*
 * Make a hexquoted copy of a string
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

const char *
hexquote_strndup(const char *src, const size_t src_len)
{
	const size_t dst_size = 4 * src_len + 1;
	assert(dst_size > src_len);

	char *dst = malloc(dst_size);
	if (!dst)
		perror_msg_and_fail("malloc(%zu)", dst_size);

	char *p = dst;
	size_t i;
	for (i = 0; i < src_len; ++i) {
		unsigned int c = ((const unsigned char *) src)[i];
		*(p++) = '\\';
		*(p++) = 'x';
		*(p++) = "0123456789abcdef"[c >> 4];
		*(p++) = "0123456789abcdef"[c & 0xf];
	}
	*p = '\0';

	return dst;
}
