/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define HAVE_ARCH_OLD_MMAP 1
#define HAVE_ARCH_OLD_SELECT 1
#define HAVE_ARCH_UID16_SYSCALLS 1
#define CAN_ARCH_BE_COMPAT_ON_64BIT_KERNEL 1
#ifdef WORDS_BIGENDIAN
# define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_ARMEB, 0 }
#else
# define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_ARM, 0 }
#endif
