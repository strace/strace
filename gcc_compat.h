/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_GCC_COMPAT_H
# define STRACE_GCC_COMPAT_H

# if defined __GNUC__ && defined __GNUC_MINOR__
#  define GNUC_PREREQ(maj, min)	\
	((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define GNUC_PREREQ(maj, min)	0
# endif

# if defined __clang__ && defined __clang_major__ && defined __clang_minor__
#  define CLANG_PREREQ(maj, min)	\
	((__clang_major__ << 16) + __clang_minor__ >= ((maj) << 16) + (min))
# else
#  define CLANG_PREREQ(maj, min)	0
# endif

# ifdef __GLIBC__
#  ifdef __GLIBC_MINOR__
#   define GLIBC_PREREQ_GE(maj, min)	\
	((__GLIBC__ << 16) + __GLIBC_MINOR__ >= ((maj) << 16) + (min))
#   define GLIBC_PREREQ_LT(maj, min)	\
	((__GLIBC__ << 16) + __GLIBC_MINOR__ < ((maj) << 16) + (min))
#  else /* !__GLIBC_MINOR__ */
#   define GLIBC_PREREQ_GE(maj, min)	0
#   define GLIBC_PREREQ_LT(maj, min)	1
#  endif
# else /* !__GLIBC__ */
#  define GLIBC_PREREQ_GE(maj, min)	0
#  define GLIBC_PREREQ_LT(maj, min)	0
# endif

# if !(GNUC_PREREQ(2, 0) || CLANG_PREREQ(1, 0))
#  define __attribute__(x)	/* empty */
# endif

# if GNUC_PREREQ(2, 5)
#  define ATTRIBUTE_NORETURN	__attribute__((__noreturn__))
# else
#  define ATTRIBUTE_NORETURN	/* empty */
# endif

# if GNUC_PREREQ(2, 7)
#  define ATTRIBUTE_FORMAT(args)	__attribute__((__format__ args))
#  define ATTRIBUTE_ALIGNED(arg)	__attribute__((__aligned__(arg)))
#  define ATTRIBUTE_PACKED	__attribute__((__packed__))
# else
#  define ATTRIBUTE_FORMAT(args)	/* empty */
#  define ATTRIBUTE_ALIGNED(arg)	/* empty */
#  define ATTRIBUTE_PACKED	/* empty */
# endif

# if GNUC_PREREQ(3, 0)
#  define SAME_TYPE(x, y)	__builtin_types_compatible_p(typeof(x), typeof(y))
#  define FAIL_BUILD_ON_ZERO(expr) (sizeof(int[-1 + 2 * !!(expr)]) * 0)
/* &(a)[0] is a pointer and not an array, shouldn't be treated as the same */
#  define MUST_BE_ARRAY(a) FAIL_BUILD_ON_ZERO(!SAME_TYPE((a), &(a)[0]))
# else
#  define SAME_TYPE(x, y)	0
#  define FAIL_BUILD_ON_ZERO(e) 0
#  define MUST_BE_ARRAY(a)	0
# endif

# if GNUC_PREREQ(3, 0)
#  define ATTRIBUTE_MALLOC	__attribute__((__malloc__))
# else
#  define ATTRIBUTE_MALLOC	/* empty */
# endif

# if GNUC_PREREQ(3, 1)
#  define ATTRIBUTE_NOINLINE	__attribute__((__noinline__))
# else
#  define ATTRIBUTE_NOINLINE	/* empty */
# endif

# if GNUC_PREREQ(4, 0)
#  define ATTRIBUTE_SENTINEL	__attribute__((__sentinel__))
# else
#  define ATTRIBUTE_SENTINEL	/* empty */
# endif

# if GNUC_PREREQ(4, 1)
#  define ALIGNOF(t_)	__alignof__(t_)
# else
#  define ALIGNOF(t_)	(sizeof(struct { char x_; t_ y_; }) - sizeof(t_))
# endif

# if GNUC_PREREQ(4, 3)
#  define ATTRIBUTE_ALLOC_SIZE(args)	__attribute__((__alloc_size__ args))
# else
#  define ATTRIBUTE_ALLOC_SIZE(args)	/* empty */
# endif

# if GNUC_PREREQ(7, 0)
#  define ATTRIBUTE_FALLTHROUGH	__attribute__((__fallthrough__))
# else
#  define ATTRIBUTE_FALLTHROUGH	((void) 0)
# endif

# if CLANG_PREREQ(2, 8)
#  define DIAG_PUSH_IGNORE_OVERRIDE_INIT					\
	_Pragma("clang diagnostic push");				\
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"");
#  define DIAG_POP_IGNORE_OVERRIDE_INIT					\
	_Pragma("clang diagnostic pop");
# elif GNUC_PREREQ(4, 2)
#  define DIAG_PUSH_IGNORE_OVERRIDE_INIT					\
	_Pragma("GCC diagnostic push");					\
	_Pragma("GCC diagnostic ignored \"-Woverride-init\"");
#  define DIAG_POP_IGNORE_OVERRIDE_INIT					\
	_Pragma("GCC diagnostic pop");
# else
#  define DIAG_PUSH_IGNORE_OVERRIDE_INIT	/* empty */
#  define DIAG_POP_IGNORE_OVERRIDE_INIT	/* empty */
# endif

# if GNUC_PREREQ(6, 0)
#  define DIAG_PUSH_IGNORE_TAUTOLOGICAL_COMPARE				\
	_Pragma("GCC diagnostic push");					\
	_Pragma("GCC diagnostic ignored \"-Wtautological-compare\"");
#  define DIAG_POP_IGNORE_TAUTOLOGICAL_COMPARE				\
	_Pragma("GCC diagnostic pop");
# else
#  define DIAG_PUSH_IGNORE_TAUTOLOGICAL_COMPARE	/* empty */
#  define DIAG_POP_IGNORE_TAUTOLOGICAL_COMPARE	/* empty */
# endif

#endif /* !STRACE_GCC_COMPAT_H */
