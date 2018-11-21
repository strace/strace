/*
 * Copyright (c) 2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef AUDIT_ARCH_PPC
# define AUDIT_ARCH_PPC 0x14
#endif

int
get_personality_from_syscall_info(const struct ptrace_syscall_info *sci)
{
	return sci->arch == AUDIT_ARCH_PPC;
}
