/*
 * Copyright (c) 2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_STATIC_ASSERT_H
# define STRACE_STATIC_ASSERT_H

# include "assert.h"

# if defined HAVE_STATIC_ASSERT

/* static_assert is already available */

# elif defined HAVE__STATIC_ASSERT

#  undef static_assert
#  define static_assert _Static_assert

# else /* !HAVE_STATIC_ASSERT && !HAVE__STATIC_ASSERT */

#  define static_assert(expr, message) \
	extern int (*strace_static_assert(int))[sizeof(int[2 * !!(expr) - 1])]

# endif

#endif /* !STRACE_STATIC_ASSERT_H */
