/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_IOVEC_H
# define STRACE_IOVEC_H

# include "kernel_types.h"

typedef struct {
	kernel_ulong_t iov_base;
	kernel_ulong_t iov_len;
} strace_iovec;

#endif /* STRACE_IOVEC_H */
