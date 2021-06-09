/*
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_MPERS_TYPE_H
# define STRACE_MPERS_TYPE_H

# include "macros.h"

# ifdef IN_MPERS
#  define DEF_MPERS_TYPE(args) STRINGIFY(args.h)
#  ifdef MPERS_IS_m32
#   define MPERS_PREFIX m32_
#   define MPERS_DEFS "m32_type_defs.h"
#  elif defined MPERS_IS_mx32
#   define MPERS_PREFIX mx32_
#   define MPERS_DEFS "mx32_type_defs.h"
#  endif
# else
#  define MPERS_PREFIX
#  define DEF_MPERS_TYPE(args) "empty.h"
#  ifdef IN_MPERS_BOOTSTRAP
#   define MPERS_DEFS "empty.h"
#  else
#   define MPERS_DEFS "native_defs.h"
#  endif
typedef unsigned long mpers_ptr_t;
# endif

#endif /* !STRACE_MPERS_TYPE_H */
