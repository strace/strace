/*
 * Fallback file for arch-specific definitions.
 *
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef HAVE_ARCH_GETRVAL2
# define HAVE_ARCH_GETRVAL2 0
#endif

#ifndef HAVE_ARCH_OLD_MMAP
# define HAVE_ARCH_OLD_MMAP 0
#endif

#ifndef HAVE_ARCH_OLD_MMAP_PGOFF
# define HAVE_ARCH_OLD_MMAP_PGOFF 0
#endif

#ifndef HAVE_ARCH_OLD_SELECT
# define HAVE_ARCH_OLD_SELECT 0
#endif

#ifndef HAVE_ARCH_UID16_SYSCALLS
# define HAVE_ARCH_UID16_SYSCALLS 0
#endif

#ifndef DEFAULT_PERSONALITY
# define DEFAULT_PERSONALITY 0
#endif

#ifndef SUPPORTED_PERSONALITIES
# define SUPPORTED_PERSONALITIES 1
#endif

#ifndef HAVE_ARCH_DEDICATED_ERR_REG
# define HAVE_ARCH_DEDICATED_ERR_REG 0
#endif

#ifndef CAN_ARCH_BE_COMPAT_ON_64BIT_KERNEL
# define CAN_ARCH_BE_COMPAT_ON_64BIT_KERNEL 0
#endif
