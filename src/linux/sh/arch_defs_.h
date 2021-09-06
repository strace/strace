/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define HAVE_ARCH_GETRVAL2 1
#define HAVE_ARCH_OLD_SELECT 1
#define HAVE_ARCH_UID16_SYSCALLS 1
#ifdef WORDS_BIGENDIAN
# define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_SH, 0 }
#else
# define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_SHEL, 0 }
#endif
