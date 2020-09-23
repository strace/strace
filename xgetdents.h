/*
 * Copyright (c) 2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_XGETDENTS_H
# define STRACE_XGETDENTS_H

# include "defs.h"

typedef unsigned int (*decode_dentry_head_fn)(struct tcb *, const void *);
typedef int (*decode_dentry_tail_fn)(struct tcb *, kernel_ulong_t,
				     const void *, unsigned int);

extern int
xgetdents(struct tcb *, unsigned int header_size,
	  decode_dentry_head_fn, decode_dentry_tail_fn);

#endif /* !STRACE_XGETDENTS_H */
