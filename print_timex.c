/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_timex.h"
#include <sys/timex.h>

#include "xlat/adjtimex_modes.h"
#include "xlat/adjtimex_status.h"


#define PRINT_TIMEX print_timex64
#define TIMEX_T kernel_timex64_t
#include "print_timex.h"
#undef TIMEX_T
#undef PRINT_TIMEX

#if HAVE_ARCH_TIME32_SYSCALLS

# define PRINT_TIMEX print_timex32
# define TIMEX_T kernel_timex32_t
# include "print_timex.h"
# undef TIMEX_T
# undef PRINT_TIMEX

#endif /* HAVE_ARCH_TIME32_SYSCALLS */

#ifdef SPARC64

# define PRINT_TIMEX print_sparc64_timex
# define TIMEX_T kernel_sparc64_timex_t
# include "print_timex.h"
# undef TIMEX_T
# undef PRINT_TIMEX

#endif /* SPARC64 */
