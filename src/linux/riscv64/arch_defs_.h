/*
 * Copyright (c) 2021-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * Linux kernel commit v6.5-rc7~13^2~5 introduced a regression:
 * when tracer changes syscall number to -1, the kernel fails
 * to initialize a0 with -ENOSYS and subsequently fails
 * to return the error code of the failed syscall to userspace.
 * Workaround this by initializing a0 ourselves.
 */
#define ARCH_NEEDS_SET_ERROR_FOR_SCNO_TAMPERING 1

#define PERSONALITY0_AUDIT_ARCH { AUDIT_ARCH_RISCV64, 0 }
