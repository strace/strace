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

#ifndef STRACE_MACROS_H
#define STRACE_MACROS_H

#include <stdbool.h>
#include <sys/types.h>

#include "gcc_compat.h"

#define ARRAY_SIZE(a_)	(sizeof(a_) / sizeof((a_)[0]) + MUST_BE_ARRAY(a_))

#define ARRSZ_PAIR(a_) a_, ARRAY_SIZE(a_)

#define STRINGIFY(...)		#__VA_ARGS__
#define STRINGIFY_VAL(...)	STRINGIFY(__VA_ARGS__)

#ifndef MAX
# define MAX(a, b)		(((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
# define MIN(a, b)		(((a) < (b)) ? (a) : (b))
#endif
#define CLAMP(val, min, max)	MIN(MAX(min, val), max)

#ifndef ROUNDUP
# define ROUNDUP(val_, div_) ((((val_) + (div_) - 1) / (div_)) * (div_))
#endif

#ifndef offsetofend
# define offsetofend(type_, member_)	\
	(offsetof(type_, member_) + sizeof(((type_ *)0)->member_))
#endif

static inline bool
is_filled(const char *ptr, char fill, size_t size)
{
	while (size--)
		if (*ptr++ != fill)
			return false;

	return true;
}

#define IS_ARRAY_ZERO(arr_)	\
	is_filled((const char *) (arr_), 0, sizeof(arr_) + MUST_BE_ARRAY(arr_))

#endif /* !STRACE_MACROS_H */
