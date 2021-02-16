/*
 * Check whether the kernel supports MIPS n64 syscalls.
 *
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef MIPS

int
main(void)
{
	__asm__(".set noreorder; li $a0, 0; li $v0, 5058; syscall");
	return 77;
}

#else

SKIP_MAIN_UNDEFINED("MIPS")

#endif
