/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define HAVE_ARCH_GETRVAL2 1
#define HAVE_ARCH_DEDICATED_ERR_REG 1
#define CAN_ARCH_BE_COMPAT_ON_64BIT_KERNEL 1

#ifdef WORDS_BIGENDIAN
# if defined(LINUX_MIPSN64)
#  define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_MIPS64, 0 }
# elif defined(LINUX_MIPSN32)
#  define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_MIPS64N32, 0 }
# else /* LINUX_MIPSO32 */
#  define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_MIPS, 0 }
# endif
#else /* !WORDS_BIGENDIAN */
# if defined(LINUX_MIPSN64)
#  define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_MIPSEL64, 0 }
# elif defined(LINUX_MIPSN32)
#  define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_MIPSEL64N32, 0 }
# else /* LINUX_MIPSO32 */
#  define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_MIPSEL, 0 }
# endif
#endif
