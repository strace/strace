/*
 * Copyright (c) 2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2018-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_ARCH_DEFS_H
# define STRACE_ARCH_DEFS_H

# include "arch_defs_.h"

/* Fallbacks for architecture-specific definitions.  */

# ifndef HAVE_ARCH_GETRVAL2
#  define HAVE_ARCH_GETRVAL2 0
# endif

# ifndef HAVE_ARCH_OLD_MMAP
#  define HAVE_ARCH_OLD_MMAP 0
# endif

# ifndef HAVE_ARCH_OLD_MMAP_PGOFF
#  define HAVE_ARCH_OLD_MMAP_PGOFF 0
# endif

# ifndef HAVE_ARCH_OLD_SELECT
#  define HAVE_ARCH_OLD_SELECT 0
# endif

# ifndef HAVE_ARCH_UID16_SYSCALLS
#  define HAVE_ARCH_UID16_SYSCALLS 0
# endif

# ifndef DEFAULT_PERSONALITY
#  define DEFAULT_PERSONALITY 0
# endif

# ifndef SUPPORTED_PERSONALITIES
#  define SUPPORTED_PERSONALITIES 1
# endif

# ifndef PERSONALITY_DESIGNATORS
#  if SUPPORTED_PERSONALITIES == 1
#   define PERSONALITY_DESIGNATORS { STRINGIFY_VAL(__WORDSIZE) }
#  elif SUPPORTED_PERSONALITIES == 2
#   define PERSONALITY_DESIGNATORS { "64", "32" }
#  endif
# endif

# ifndef PERSONALITY_NAMES
#  if SUPPORTED_PERSONALITIES == 1
#   define PERSONALITY_NAMES { STRINGIFY_VAL(__WORDSIZE) " bit" }
#  elif SUPPORTED_PERSONALITIES == 2
#   define PERSONALITY_NAMES { "64 bit", "32 bit" }
#  endif
# endif

# ifndef HAVE_ARCH_DEDICATED_ERR_REG
#  define HAVE_ARCH_DEDICATED_ERR_REG 0
# endif

# ifndef CAN_ARCH_BE_COMPAT_ON_64BIT_KERNEL
#  define CAN_ARCH_BE_COMPAT_ON_64BIT_KERNEL 0
# endif

# ifndef ARCH_NEEDS_SET_ERROR_FOR_SCNO_TAMPERING
#  define ARCH_NEEDS_SET_ERROR_FOR_SCNO_TAMPERING 0
# endif

# ifndef ARCH_NEEDS_NON_SHUFFLED_SCNO_CHECK
#  define ARCH_NEEDS_NON_SHUFFLED_SCNO_CHECK 0
# endif

# ifndef MIN_WORDSIZE
#  if SUPPORTED_PERSONALITIES > 1
#   define MIN_WORDSIZE 4
#  else
#   define MIN_WORDSIZE SIZEOF_LONG
#  endif
# endif

# ifndef HAVE_ARCH_TIME32_SYSCALLS
#  define HAVE_ARCH_TIME32_SYSCALLS (MIN_WORDSIZE == 4)
# endif

# ifndef HAVE_ARCH_OLD_TIME64_SYSCALLS
#  define HAVE_ARCH_OLD_TIME64_SYSCALLS (SIZEOF_LONG == 8)
# endif

# ifndef MIN_KLONGSIZE
#  if SUPPORTED_PERSONALITIES > 1
#   define MIN_KLONGSIZE 4
#  else
#   define MIN_KLONGSIZE SIZEOF_KERNEL_LONG_T
#  endif
# endif

# ifndef HAVE_ARCH_TIMESPEC32
#  define HAVE_ARCH_TIMESPEC32 (MIN_KLONGSIZE == 4)
# endif

#endif /* !STRACE_ARCH_DEFS_H */
