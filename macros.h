/*
 * Copyright (c) 2001-2019 The strace developers.
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
# define CLAMP(val, min, max)	MIN(MAX(min, val), max)

# ifndef ROUNDUP_DIV
#  define ROUNDUP_DIV(val_, div_) (((val_) + (div_) - 1) / (div_))
# endif

# ifndef ROUNDUP
#  define ROUNDUP(val_, div_) (ROUNDUP_DIV((val_), (div_)) * (div_))
# endif

# define sizeof_field(type_, member_) (sizeof(((type_ *)0)->member_))

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

# ifndef BIT
#  define BIT(x_) (1U << (x_))
# endif

# define FLAG(name_) name_ = BIT(name_##_BIT)

#endif /* !STRACE_MACROS_H */
