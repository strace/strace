/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_NEGATED_ERRNO_H
# define STRACE_NEGATED_ERRNO_H

/*
 * Check the syscall return value register value for whether it is
 * a negated errno code indicating an error, or a success return value.
 */
static inline bool
is_negated_errno(kernel_ulong_t val)
{
	kernel_ulong_t max = -(kernel_long_t) MAX_ERRNO_VALUE;

# ifndef current_klongsize
	if (current_klongsize < sizeof(val)) {
		val = (uint32_t) val;
		max = (uint32_t) max;
	}
# endif /* !current_klongsize */

	return val >= max;
}

#endif /* !STRACE_NEGATED_ERRNO_H */
