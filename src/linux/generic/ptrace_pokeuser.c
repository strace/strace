/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static long
ptrace_pokeuser(int pid, unsigned long off, kernel_ulong_t val)
{
	return ptrace(PTRACE_POKEUSER, pid, off, val);
}
