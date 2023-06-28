/*
 * Copyright (c) 2001-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_MACROS_H
# define STRACE_MACROS_H

# include <stdbool.h>
# include <stddef.h>
# include <sys/types.h>

# include "gcc_compat.h"
# include "static_assert.h"

/*
 * Evaluates to:
 * a syntax error, if the argument is 0;
 * 0, otherwise.
 */
# define FAIL_BUILD_ON_ZERO(e_)	(sizeof(int[-1 + 2 * !!(e_)]) * 0)

/*
 * Evaluates to:
 * 1, if the given type is known to be a non-array type;
 * 0, otherwise.
 */
# define IS_NOT_ARRAY(a_)	IS_SAME_TYPE((a_), &(a_)[0])

/*
 * Evaluates to:
 * a syntax error, if the argument is not an array;
 * 0, otherwise.
 */
# define MUST_BE_ARRAY(a_)	FAIL_BUILD_ON_ZERO(!IS_NOT_ARRAY(a_))

/* Evaluates to the number of elements in the specified array.  */
# define ARRAY_SIZE(a_)	(sizeof(a_) / sizeof((a_)[0]) + MUST_BE_ARRAY(a_))

# define ARRSZ_PAIR(a_) a_, ARRAY_SIZE(a_)

# define STRINGIFY(...)		#__VA_ARGS__
# define STRINGIFY_VAL(...)	STRINGIFY(__VA_ARGS__)

# ifndef MAX
#  define MAX(a, b)		(((a) > (b)) ? (a) : (b))
# endif
# ifndef MIN
#  define MIN(a, b)		(((a) < (b)) ? (a) : (b))
# endif
# define CLAMP(val, min, max)	MIN(MAX(val, min), max)

# ifndef ROUNDUP_DIV
#  define ROUNDUP_DIV(val_, div_) (((val_) + (div_) - 1) / (div_))
# endif

# ifndef ROUNDUP
#  define ROUNDUP(val_, div_) (ROUNDUP_DIV((val_), (div_)) * (div_))
# endif

# define sizeof_field(type_, member_) (sizeof(((type_ *)0)->member_))

# define typeof_field(type_, member_) typeof(((type_ *)0)->member_)

# ifndef offsetofend
#  define offsetofend(type_, member_)	\
	(offsetof(type_, member_) + sizeof_field(type_, member_))
# endif

# ifndef cast_ptr
#  define cast_ptr(type_, var_)	\
	((type_) (uintptr_t) (const volatile void *) (var_))
# endif

# ifndef containerof
/**
 * Return a pointer to a structure that contains the provided variable.
 *
 * @param ptr_    Pointer to data that is a field of the container structure.
 * @param struct_ Type of the container structure.
 * @param member_ Name of the member field.
 * @return  Pointer to the container structure.
 */
#  define containerof(ptr_, struct_, member_)	\
	cast_ptr(struct_ *,			\
		 (const volatile char *) (ptr_) - offsetof(struct_, member_))
# endif

static inline bool
is_filled(const char *ptr, char fill, size_t size)
{
	while (size--)
		if (*ptr++ != fill)
			return false;

	return true;
}

# define IS_ARRAY_ZERO(arr_)	\
	is_filled((const char *) (arr_), 0, sizeof(arr_) + MUST_BE_ARRAY(arr_))

# ifndef BIT32
#  define BIT32(x_) (1U << (x_))
# endif

# ifndef BIT64
#  define BIT64(x_) (1ULL << (x_))
# endif

# ifndef MASK32
#  define MASK32(x_) (BIT32(x_) - 1U)
# endif

# ifndef MASK64
#  define MASK64(x_) (BIT64(x_) - 1ULL)
# endif

/*
 * "Safe" versions that avoid UB for values that are >= type bit size
 * (the usually expected behaviour of the bit shift in that case is zero,
 * but at least powerpc is notorious for returning the input value when shift
 * by 64 bits is performed).
 */

# define BIT32_SAFE(x_) ((x_) < 32 ? BIT32(x_) : 0)
# define BIT64_SAFE(x_) ((x_) < 64 ? BIT64(x_) : 0)
# define MASK32_SAFE(x_) (BIT32_SAFE(x_) - 1U)
# define MASK64_SAFE(x_) (BIT64_SAFE(x_) - 1ULL)

# define FLAG(name_) name_ = BIT32(name_##_BIT)

/**
 * A shorthand for a build-time check of a type size that provides
 * a corresponding "update the decoder" message in a case of failure.
 * @param type_ Type whose size is to be checked.
 * @param sz_   Expected type size in bytes.
 */
# define CHECK_TYPE_SIZE(type_, sz_) \
	static_assert(sizeof(type_) == (sz_), \
		      "Unexpected size of " #type_ "(" #sz_ " expected)")

/** Checks that ioctl code's size field contains the expected value. */
# define CHECK_IOCTL_SIZE(ioc_, sz_) \
	static_assert(_IOC_SIZE(ioc_) == (sz_), \
		"Unexpected size field value in " #ioc_ " (" #sz_" expected)")

# ifdef WORDS_BIGENDIAN
#  define BE16(val_) val_
#  define BE32(val_) val_
#  define BE64(val_) val_
# else
#  define BE16(val_) ((((val_) & 0xff) << 8) | (((val_) >> 8) & 0xff))
#  define BE32(val_) ((BE16(val_) << 16) | BE16((val_) >> 16))
#  define BE64(val_) ((BE32(val_) << 32) | BE32((val_) >> 32))
# endif

#endif /* !STRACE_MACROS_H */
