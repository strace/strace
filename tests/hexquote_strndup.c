/*
 * Make a hexquoted copy of a string
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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
