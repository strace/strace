/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static const char *
nr_prefix(kernel_ulong_t scno)
{
	/*
	 * For now, the set of syscalls that are shuffled is equivalent to the
	 * set of syscalls that have __ARM_NR_ prefix.
	 */
	if (shuffle_scno(scno) != scno)
		return "__ARM_NR_";
	else
		return "__NR_";
}
