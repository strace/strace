/*
 * Copyright (c) 2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

int
get_personality_from_syscall_info(const struct_ptrace_syscall_info *sci)
{
	return sci->arch == AUDIT_ARCH_S390;
}
