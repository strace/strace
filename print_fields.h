/*
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@altlinux.org>
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

#ifndef STRACE_PRINT_FIELDS_H
#define STRACE_PRINT_FIELDS_H

/*
 * The printf-like function to use in header files
 * shared between strace and its tests.
 */
#ifndef STRACE_PRINTF
# define STRACE_PRINTF tprintf
#endif

#define PRINT_FIELD_D(prefix_, where_, field_)				\
	STRACE_PRINTF("%s%s=%lld", (prefix_), #field_,			\
		      sign_extend_unsigned_to_ll((where_).field_))

#define PRINT_FIELD_U(prefix_, where_, field_)				\
	STRACE_PRINTF("%s%s=%llu", (prefix_), #field_,			\
		      zero_extend_signed_to_ull((where_).field_))

#define PRINT_FIELD_X(prefix_, where_, field_)				\
	STRACE_PRINTF("%s%s=%#llx", (prefix_), #field_,			\
		      zero_extend_signed_to_ull((where_).field_))

#define PRINT_FIELD_COOKIE(prefix_, where_, field_)			\
	STRACE_PRINTF("%s%s=[%llu, %llu]", (prefix_), #field_,		\
		      zero_extend_signed_to_ull((where_).field_[0]),	\
		      zero_extend_signed_to_ull((where_).field_[1]))

#define PRINT_FIELD_FLAGS(prefix_, where_, field_, xlat_, dflt_)	\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printflags((xlat_), (where_).field_, (dflt_));		\
	} while (0)

#define PRINT_FIELD_XVAL(prefix_, where_, field_, xlat_, dflt_)		\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printxval((xlat_), (where_).field_, (dflt_));		\
	} while (0)

#endif /* !STRACE_PRINT_FIELDS_H */
