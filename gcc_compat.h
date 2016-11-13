/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifndef STRACE_GCC_COMPAT_H
#define STRACE_GCC_COMPAT_H

#if defined __GNUC__ && defined __GNUC_MINOR__
# define GNUC_PREREQ(maj, min)	\
	((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define __attribute__(x)	/* empty */
# define GNUC_PREREQ(maj, min)	0
#endif

#if GNUC_PREREQ(2, 5)
# define ATTRIBUTE_NORETURN	__attribute__((__noreturn__))
#else
# define ATTRIBUTE_NORETURN	/* empty */
#endif

#if GNUC_PREREQ(2, 7)
# define ATTRIBUTE_FORMAT(args)	__attribute__((__format__ args))
# define ATTRIBUTE_ALIGNED(arg)	__attribute__((__aligned__(arg)))
# define ATTRIBUTE_PACKED	__attribute__((__packed__))
#else
# define ATTRIBUTE_FORMAT(args)	/* empty */
# define ATTRIBUTE_ALIGNED(arg)	/* empty */
# define ATTRIBUTE_PACKED	/* empty */
#endif

#if GNUC_PREREQ(3, 0)
# define SAME_TYPE(x, y)	__builtin_types_compatible_p(typeof(x), typeof(y))
# define FAIL_BUILD_ON_ZERO(expr) (sizeof(int[-1 + 2 * !!(expr)]) * 0)
/* &(a)[0] is a pointer and not an array, shouldn't be treated as the same */
# define MUST_BE_ARRAY(a) FAIL_BUILD_ON_ZERO(!SAME_TYPE((a), &(a)[0]))
#else
# define SAME_TYPE(x, y)	0
# define MUST_BE_ARRAY(a)	0
#endif

#if GNUC_PREREQ(3, 0)
# define ATTRIBUTE_MALLOC	__attribute__((__malloc__))
#else
# define ATTRIBUTE_MALLOC	/* empty */
#endif

#if GNUC_PREREQ(3, 1)
# define ATTRIBUTE_NOINLINE	__attribute__((__noinline__))
#else
# define ATTRIBUTE_NOINLINE	/* empty */
#endif

#if GNUC_PREREQ(4, 0)
# define ATTRIBUTE_SENTINEL	__attribute__((__sentinel__))
#else
# define ATTRIBUTE_SENTINEL	/* empty */
#endif

#if GNUC_PREREQ(4, 1)
# define ALIGNOF(t_)	__alignof__(t_)
#else
# define ALIGNOF(t_)	(sizeof(struct {char x_; t_ y_;}) - sizeof(t_))
#endif

#if GNUC_PREREQ(4, 3)
# define ATTRIBUTE_ALLOC_SIZE(args)	__attribute__((__alloc_size__ args))
#else
# define ATTRIBUTE_ALLOC_SIZE(args)	/* empty */
#endif

#endif /* !STRACE_GCC_COMPAT_H */
