/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_OPENAT2_H
# define STRACE_TYPES_OPENAT2_H

# ifdef HAVE_LINUX_OPENAT2_H
#  include <linux/openat2.h>
# endif

typedef struct {
	uint64_t flags;
	uint64_t mode;
	uint64_t resolve;
} struct_open_how;

#endif /* STRACE_TYPES_OPENAT2_H */
