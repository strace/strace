/*
 * Copyright (c) 2001-2018 The strace developers.
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

#ifndef STRACE_STRING_TO_UINT_H
#define STRACE_STRING_TO_UINT_H

#include <limits.h>

#include "kernel_types.h"

extern long long
string_to_uint_ex(const char *str, char **endptr,
		  unsigned long long max_val, const char *accepted_ending);

static inline long long
string_to_uint_upto(const char *const str, const unsigned long long max_val)
{
	return string_to_uint_ex(str, NULL, max_val, NULL);
}

static inline int
string_to_uint(const char *str)
{
	return string_to_uint_upto(str, INT_MAX);
}

static inline long
string_to_ulong(const char *str)
{
	return string_to_uint_upto(str, LONG_MAX);
}

static inline kernel_long_t
string_to_kulong(const char *str)
{
	return string_to_uint_upto(str, ((kernel_ulong_t) -1ULL) >> 1);
}

static inline long long
string_to_ulonglong(const char *str)
{
	return string_to_uint_upto(str, LLONG_MAX);
}

#endif /* !STRACE_STRING_TO_UINT_H */
